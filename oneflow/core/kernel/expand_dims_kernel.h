#ifndef ONEFLOW_CORE_KERNEL_EXPAND_DIMS_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_EXPAND_DIMS_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
#include "oneflow/core/kernel/kernel_context.h"

namespace oneflow {

template<DeviceType device_type>
class ExpandDimsKernel final : public KernelIf<device_type> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ExpandDimsKernel);
  ExpandDimsKernel() = default;
  ~ExpandDimsKernel() = default;

 private:
  void ForwardDataContent(const KernelCtx&,
                          std::function<Blob*(const std::string&)>) const override;
  void BackwardDataContent(const KernelCtx&,
                           std::function<Blob*(const std::string&)>) const override;
  void ForwardInstanceShape(const KernelCtx&,
                            std::function<Blob*(const std::string&)>) const override;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_EXPAND_DIMS_KERNEL_H_