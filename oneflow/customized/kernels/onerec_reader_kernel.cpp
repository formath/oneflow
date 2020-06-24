#include "oneflow/core/framework/framework.h"
#include "oneflow/customized/data/onerec_data_reader.h"

namespace oneflow {

namespace {

class OneRecReaderWrapper final : public user_op::OpKernelState {
 public:
  explicit OneRecReaderWrapper(user_op::KernelInitContext* ctx) : reader_(ctx) {}
  ~OneRecReaderWrapper() = default;

  void Read(user_op::KernelComputeContext* ctx) { reader_.Read(ctx); }

 private:
  data::OneRecDataReader reader_;
};

}  // namespace

class OneRecReaderKernel final : public user_op::OpKernel {
 public:
  OneRecReaderKernel() = default;
  ~OneRecReaderKernel() override = default;

 private:
  std::shared_ptr<user_op::OpKernelState> CreateOpKernelState(
      user_op::KernelInitContext* ctx) const override {
    std::shared_ptr<OneRecReaderWrapper> reader(new OneRecReaderWrapper(ctx));
    return reader;
  }

  void Compute(user_op::KernelComputeContext* ctx, user_op::OpKernelState* state) const override {
    auto* reader = dynamic_cast<OneRecReaderWrapper*>(state);
    reader->Read(ctx);
  }
  bool AlwaysComputeWhenAllOutputsEmpty() const override { return false; }
};

REGISTER_USER_KERNEL("OneRecReader")
    .SetCreateFn<OneRecReaderKernel>()
    .SetIsMatchedHob(user_op::HobDeviceType() == DeviceType::kCPU
                     & user_op::HobDataType("out", 0) == DataType::kTensorBuffer);

}  // namespace oneflow