#include "oneflow/core/graph/collective_boxing_task_node.h"

namespace oneflow {

void CollectiveBoxingGenericTaskNode::Init(int64_t machine_id, int64_t thrd_id, int64_t area_id,
                                           const OperatorConf& op_conf) {
  set_area_id(area_id);
  set_machine_id(machine_id);
  set_thrd_id(thrd_id);
  op_conf_ = op_conf;
}

void CollectiveBoxingGenericTaskNode::ProduceAllRegstsAndBindEdges() {
  std::shared_ptr<RegstDesc> out_regst = ProduceRegst("out", false, 1, 1);
  this->ForEachOutDataEdge(
      [&](TaskEdge* out_dege) { this->SoleOutDataEdge()->AddRegst("out", out_regst); });
}

void CollectiveBoxingGenericTaskNode::ConsumeAllRegsts() {
  this->ForEachInDataEdge(
      [&](TaskEdge* in_edge) { ConsumeRegst("in", SoleInDataEdge()->GetSoleRegst()); });
}

void CollectiveBoxingGenericTaskNode::BuildExecGphAndRegst() {
  ExecNode* node = mut_exec_gph().NewNode();
  std::shared_ptr<Operator> boxing_op = ConstructOp(op_conf_, &GlobalJobDesc());
  node->mut_op() = boxing_op;
  for (const std::string& ibn : boxing_op->input_bns()) {
    node->BindBnWithRegst(ibn, GetSoleConsumedRegst("in"));
  }
  std::shared_ptr<RegstDesc> out_regst = GetProducedRegst("out");
  for (const std::string& obn : boxing_op->output_bns()) {
    node->BindBnWithRegst(obn, out_regst);
    out_regst->AddLbi(boxing_op->BnInOp2Lbi(obn));
  }
  node->InferBlobDescs(nullptr);
}

void CollectiveBoxingGenericTaskNode::InferProducedDataRegstTimeShape() {
  auto out_regst = GetProducedRegst("out");
  if (out_regst != nullptr) {
    out_regst->mut_data_regst_time_shape()->reset(
        new Shape({GlobalJobDesc().TotalBatchNum(), GlobalJobDesc().NumOfPiecesInBatch()}));
  }
}

}  // namespace oneflow