#ifndef ONEFLOW_CORE_OPERATOR_LOSS_RECORD_H_
#define ONEFLOW_CORE_OPERATOR_LOSS_RECORD_H_

#include "oneflow/core/operator/operator_manager.h"

namespace oneflow {

class LossRecordOp final : public SysOperator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(LossRecordOp);
  LossRecordOp() = default;
  ~LossRecordOp() = default;

  void InitFromOpConf(const OperatorConf& op_conf) override;
  const PbMessage& GetSpecialConf() const override;

 private:
  std::string ibn2lbn(const std::string& input_bn) const override {
    return kPackedBlobName;
  }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_LOSS_RECORD_H_
