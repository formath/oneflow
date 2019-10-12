from __future__ import absolute_import

import oneflow as flow
import oneflow.core.operator.op_conf_pb2 as op_conf_util
import oneflow.core.register.logical_blob_id_pb2 as logical_blob_id_util
import oneflow.python.framework.id_util as id_util
import oneflow.python.framework.compile_context as compile_context
import oneflow.python.framework.remote_blob as remote_blob_util
import oneflow.python.framework.distribute as distribute_util
from oneflow.python.oneflow_export import oneflow_export


@oneflow_export("layers.dense")
def dense(
    inputs,
    units,
    activation=None,
    use_bias=True,
    kernel_initializer=None,
    bias_initializer=None,
    trainable=True,
    name=None,
    model_distribute=distribute_util.broadcast(),
    primary_lr=None,
):
    in_shape = inputs.static_shape
    in_num_axes = len(in_shape)
    assert in_num_axes >= 2

    name_prefix = name if name is not None else id_util.UniqueStr("Dense_")
    inputs = flow.reshape(inputs, (-1, in_shape[-1])) if in_num_axes > 2 else inputs

    assert model_distribute is distribute_util.auto() or \
        model_distribute is distribute_util.broadcast() or \
        model_distribute is distribute_util.split(0)

    if model_distribute is distribute_util.split(0):
        assert in_num_axes is 2 # model distribute is hard for reshape split dim 1

    weight = flow.get_variable(
        name="{}-weight".format(name_prefix),
        shape=(units, inputs.static_shape[1]),
        dtype=inputs.dtype,
        initializer=(
            kernel_initializer
            if kernel_initializer is not None
            else flow.constant_initializer(0)
        ),
        trainable=trainable,
        model_name="weight",
        primary_lr=primary_lr,
        distribute=model_distribute)
    weight = weight.with_distribute(model_distribute)

    out = flow.matmul(
        a=inputs, b=weight, transpose_b=True, name="{}_matmul".format(name_prefix)
    )
    if use_bias:
        bias = flow.get_variable(
            name="{}-bias".format(name_prefix),
            shape=(units,),
            dtype=inputs.dtype,
            initializer=(
                bias_initializer
                if bias_initializer is not None
                else flow.constant_initializer(0)
            ),
            trainable=trainable,
            model_name="bias",
            distribute=model_distribute)
        bias = bias.with_distribute(model_distribute)
        out = flow.nn.bias_add(out, bias, name="{}_bias_add".format(name_prefix))
    out = activation(out) if activation is not None else out
    out = flow.reshape(out, in_shape[:-1] + (units,)) if in_num_axes > 2 else out

    return out


@oneflow_export("layers.layer_norm")
def layer_norm(
    inputs,
    center=True,
    scale=True,
    trainable=True,
    begin_norm_axis=1,
    begin_params_axis=-1,
    name=None,
):
    op_conf = op_conf_util.OperatorConf()
    name = name if name is not None else id_util.UniqueStr(
        "LayerNorm_")
    begin_params_axis = begin_params_axis if begin_params_axis >= 0 else len(
        inputs.shape) + begin_params_axis
    param_shape = inputs.shape[begin_params_axis:]
    if len(param_shape) is 0:
        param_shape = (1,)
    if center:
        beta = flow.get_variable(
            name="{}-beta".format(name),
            shape=param_shape,
            dtype=inputs.dtype,
            initializer=flow.constant_initializer(0.0),
            trainable=trainable,
            model_name="weight",
            distribute=distribute_util.broadcast(),
        )
        setattr(op_conf.layer_norm_conf, "beta", beta.logical_blob_name)
    if scale:
        gamma = flow.get_variable(
            name="{}-gamma".format(name),
            shape=param_shape,
            dtype=inputs.dtype,
            initializer=flow.constant_initializer(1.0),
            trainable=trainable,
            model_name="weight",
            distribute=distribute_util.broadcast(),
        )
        setattr(op_conf.layer_norm_conf, "gamma", gamma.logical_blob_name)
    setattr(op_conf, "name", name)
    setattr(op_conf, "trainable", trainable)
    setattr(op_conf.layer_norm_conf, "in", inputs.logical_blob_name)
    setattr(op_conf.layer_norm_conf, "out", "out")
    setattr(op_conf.layer_norm_conf, "center", center)
    setattr(op_conf.layer_norm_conf, "scale", scale)
    setattr(op_conf.layer_norm_conf, "begin_norm_axis", begin_norm_axis)
    setattr(op_conf.layer_norm_conf, "begin_params_axis", begin_params_axis)
    compile_context.CurJobAddOp(op_conf)
    out_lbi = logical_blob_id_util.LogicalBlobId()
    setattr(out_lbi, "op_name", op_conf.name)
    setattr(out_lbi, "blob_name", "out")
    return remote_blob_util.RemoteBlob(out_lbi)


@oneflow_export("layers.batch_normalization")
def batch_normalization( 
    inputs,
    axis=-1,
    momentum=0.99,
    epsilon=0.001,
    center=True,
    scale=True,
    beta_initializer=None,
    gamma_initializer=None,
    moving_mean_initializer=None,
    moving_variance_initializer=None,
    trainable=False,
    is_training=True,
    name=None,
):
    assert axis >= -len(inputs.shape) and axis < len(inputs.shape)
    params_shape = [inputs.shape[axis]]

    if name is None:
        name = id_util.UniqueStr("BatchNorm_")

    if center:
        beta = flow.get_variable(
            name=name + "-beta",
            shape=params_shape,
            dtype=inputs.dtype,
            initializer=beta_initializer or flow.zeros_initializer(),
            trainable=trainable,
            distribute=distribute_util.broadcast(),
        )

    if scale:
        gamma = flow.get_variable(
            name=name + "-gamma",
            shape=params_shape,
            dtype=inputs.dtype,
            initializer=gamma_initializer or flow.ones_initializer(),
            trainable=trainable,
            distribute=distribute_util.broadcast(),
        )

    moving_mean = flow.get_variable(
        name=name + "-moving_mean",
        shape=params_shape,
        dtype=inputs.dtype,
        initializer=moving_mean_initializer or flow.zeros_initializer(),
        trainable=trainable,
        distribute=distribute_util.broadcast(),
    )

    moving_variance = flow.get_variable(
        name=name + "-moving_variance",
        shape=params_shape,
        dtype=inputs.dtype,
        initializer=moving_variance_initializer or flow.ones_initializer(),
        trainable=trainable,
        distribute=distribute_util.broadcast(),
    )

    op_conf = op_conf_util.OperatorConf()
    setattr(op_conf, "name", name)
    setattr(op_conf.normalization_conf, "in", inputs.logical_blob_name)
    setattr(op_conf.normalization_conf, "out", "out")
    setattr(op_conf.normalization_conf, "axis", axis)
    setattr(op_conf.normalization_conf, "momentum", momentum)
    setattr(op_conf.normalization_conf, "epsilon", epsilon)
    setattr(op_conf.normalization_conf, "center", center)
    setattr(op_conf.normalization_conf, "scale", scale)
    setattr(
        op_conf.normalization_conf, "moving_mean", moving_mean.logical_blob_name
    )
    setattr(
        op_conf.normalization_conf,
        "moving_variance",
        moving_variance.logical_blob_name,
    )
    if beta:
        setattr(op_conf.normalization_conf, "beta", beta.logical_blob_name)
    if gamma:
        setattr(op_conf.normalization_conf, "gamma", gamma.logical_blob_name)
    if trainable:
        setattr(op_conf.normalization_conf, "mean", "mean")
        setattr(op_conf.normalization_conf, "inv_variance", "inv_variance")
        setattr(op_conf.normalization_conf, "is_training", is_training)
    else:
        setattr(op_conf.normalization_conf, "is_training", False)

    compile_context.CurJobAddOp(op_conf)
    out_lbi = logical_blob_id_util.LogicalBlobId()
    setattr(out_lbi, "op_name", op_conf.name)
    setattr(out_lbi, "blob_name", "out")
    return remote_blob_util.RemoteBlob(out_lbi)

@oneflow_export("layers.PRelu")
def prelu(
    inputs,
    alpha_initializer,
    data_format,
    channel_shared,
    name=None,
    model_distribute=distribute_util.broadcast(),
):
  if channel_shared:
    alpha_shape = [1]
  else:
    if data_format == "channels_first":
      alpha_shape = [inputs.shape[1]]
    elif data_format == "channels_last":
      alpha_shape = [inputs.shape[-1]]
    else:
      raise ValueError("invalid data_format")
    alpha = flow.get_variable(
        name + "-alpha",
        shape=alpha_shape,
        dtype=inputs.dtype,
        initializer=flow.constant_initializer(alpha_initializer),
        distribute=model_distribute
    )
    op_conf = op_conf_util.OperatorConf()
    setattr(op_conf, "name", name)
    setattr(op_conf.prelu_conf, "in", inputs.logical_blob_name)
    setattr(op_conf.prelu_conf, "out", "out")
    setattr(op_conf.prelu_conf, "alpha", alpha.logical_blob_name)
    setattr(op_conf.prelu_conf, "data_format", data_format)
    setattr(op_conf.prelu_conf, "channel_shared", channel_shared)
    compile_context.CurJobAddOp(op_conf)
    out_lbi = logical_blob_id_util.LogicalBlobId()
    setattr(out_lbi, "op_name", op_conf.name)
    setattr(out_lbi, "blob_name", "out")
    return remote_blob_util.RemoteBlob(out_lbi) 