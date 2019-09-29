#ifndef ONEFLOW_CORE_OPERATOR_RESIZE_NEAREST_NEIGHBOR_OP_H_
#define ONEFLOW_CORE_OPERATOR_RESIZE_NEAREST_NEIGHBOR_OP_H_

#include "oneflow/core/operator/operator.h"
#include "oneflow/core/operator/operator_util.h"

namespace oneflow {

class ResizeNearestNeighborOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ResizeNearestNeighborOp);
  ResizeNearestNeighborOp() = default;
  ~ResizeNearestNeighborOp() = default;

  void InitFromOpConf() override;
  const PbMessage& GetCustomizedConf() const override;
  bool NeedInBlobWhenBackward() const override { return false; }
  bool NeedOutBlobWhenBackward() const override { return false; }
  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

 private:
  bool IsInputBlobAllowedModelSplit(const std::string& ibn) const override { return false; }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_RESIZE_NEAREST_NEIGHBOR_OP_H_