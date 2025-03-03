#include <torch/csrc/jit/passes/subgraph_rewrite.h>

#include "core/util/prelude.h"

namespace torch_tensorrt {
namespace core {
namespace lowering {
namespace passes {

void ReduceToOperation(std::shared_ptr<torch::jit::Graph>& graph) {
  std::string to_device_pattern = R"IR(
        graph(%x, %device, %dtype, %nb, %copy, %format):
            %out : Tensor = aten::to(%x, %device, %dtype, %nb, %copy, %format)
            return (%out))IR";
  std::string to_dtype_pattern = R"IR(
        graph(%x, %device, %dtype, %nb, %copy, %format):
            %out : Tensor = aten::to(%x, %dtype, %nb, %copy, %format)
            return (%out))IR";

  std::string to_type_as_pattern = R"IR(
        graph(%input, %other):
            %out : Tensor = aten::type_as(%input, %other)
            return (%out))IR";

  std::string to_other_pattern = R"IR(
        graph(%input, %other):
            %5 : bool = prim::Constant[value=0]()
            %6 : None = prim::Constant()
            %out : Tensor = aten::to(%input, %other, %5, %5, %6)
            return (%out))IR";

  // replace aten::to.device with aten::to.dtype
  torch::jit::SubgraphRewriter map_aten_device_to_dtype;
  map_aten_device_to_dtype.RegisterRewritePattern(to_device_pattern, to_dtype_pattern);
  map_aten_device_to_dtype.runOnGraph(graph);

  // replace aten::type_as with aten::to.other
  torch::jit::SubgraphRewriter map_aten_type_as_to_other;
  map_aten_type_as_to_other.RegisterRewritePattern(to_type_as_pattern, to_other_pattern);
  map_aten_type_as_to_other.runOnGraph(graph);

  LOG_GRAPH("Post lowering of [aten::to.device|aten::type_as] -> " << *graph);
}

} // namespace passes
} // namespace lowering
} // namespace core
} // namespace torch_tensorrt
