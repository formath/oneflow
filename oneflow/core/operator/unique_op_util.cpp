#include "oneflow/core/operator/unique_op_util.h"
#include "oneflow/core/kernel/unique_kernel_util.h"

namespace oneflow {

namespace {

template<DeviceType device_type, typename T, typename U>
void GetUniqueWorkspaceSizeInBytes(int64_t n, int64_t* workspace_size_in_bytes) {
  UniqueKernelUtil<device_type, T, U>::GetUniqueWorkspaceSizeInBytes(nullptr, n,
                                                                     workspace_size_in_bytes);
}

struct SwitchUtil final {
#define SWITCH_ENTRY(func_name, device_type, T, U) func_name<device_type, T, U>
#define UNIQUE_KV_DATA_TYPE_SEQ                   \
  OF_PP_MAKE_TUPLE_SEQ(int32_t, DataType::kInt32) \
  OF_PP_MAKE_TUPLE_SEQ(int64_t, DataType::kInt64)
  DEFINE_STATIC_SWITCH_FUNC(void, GetUniqueWorkspaceSizeInBytes, SWITCH_ENTRY,
                            MAKE_DEVICE_TYPE_CTRV_SEQ(DEVICE_TYPE_SEQ),
                            MAKE_DATA_TYPE_CTRV_SEQ(UNIQUE_KV_DATA_TYPE_SEQ),
                            MAKE_DATA_TYPE_CTRV_SEQ(UNIQUE_KV_DATA_TYPE_SEQ));
#undef UNIQUE_KV_DATA_TYPE_SEQ
#undef SWITCH_ENTRY
};

}  // namespace

void UniqueOpUtil::GetUniqueWorkspaceSizeInBytes(DeviceType device_type, DataType value_type,
                                                 DataType index_type, int64_t n,
                                                 int64_t* workspace_size_in_bytes) {
  SwitchUtil::SwitchGetUniqueWorkspaceSizeInBytes(SwitchCase(device_type, value_type, index_type),
                                                  n, workspace_size_in_bytes);
}

}  // namespace oneflow