#ifndef ONEFLOW_CORE_ACTOR_MODEL_DIFF_ACCUMULATE_ACTOR_H_
#define ONEFLOW_CORE_ACTOR_MODEL_DIFF_ACCUMULATE_ACTOR_H_

#include "oneflow/core/actor/accumulate_actor.h"

namespace oneflow {

class MdDiffAccActor final : public AccumulateActor {
 public:
  OF_DISALLOW_COPY_AND_MOVE(MdDiffAccActor);
  MdDiffAccActor() = default;
  ~MdDiffAccActor() = default;

  void Init(const TaskProto& proto, const ThreadCtx& ctx) override {
    AccumulateActor::Init(proto, ctx,
                          JobDesc::Singleton()->num_of_pieces_in_batch());
  }

 private:
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_ACTOR_MODEL_DIFF_ACCUMULATE_ACTOR_H_
