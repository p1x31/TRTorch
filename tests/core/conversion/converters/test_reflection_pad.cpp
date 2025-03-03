#include <iostream>
#include <string>
#include "core/compiler.h"
#include "gtest/gtest.h"
#include "tests/util/util.h"
#include "torch/csrc/jit/ir/irparser.h"

TEST(Converters, ATenReflection_pad2dTensorConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor):
        %1 : int = prim::Constant[value=1]()
        %2 : int = prim::Constant[value=1]()
        %3 : int = prim::Constant[value=2]()
        %4 : int = prim::Constant[value=0]()
        %5 : int[] = prim::ListConstruct(%1, %2, %3, %4)
        %6 : Tensor = aten::reflection_pad2d(%0, %5)
        return (%6))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {1, 3, 5, 5}, {at::kCUDA});

  auto params = torch_tensorrt::core::conversion::get_named_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1});

  params = torch_tensorrt::core::conversion::get_named_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0]), 2e-6));
}

TEST(Converters, ATenReflection_pad1dTensorConvertsCorrectly) {
  const auto graph = R"IR(
      graph(%0 : Tensor):
        %1 : int = prim::Constant[value=1]()
        %2 : int = prim::Constant[value=2]()
        %3 : int[] = prim::ListConstruct(%1, %2)
        %4 : Tensor = aten::reflection_pad1d(%0, %3)
        return (%4))IR";

  auto g = std::make_shared<torch::jit::Graph>();
  torch::jit::parseIR(graph, g.get());

  auto in1 = at::randint(1, 10, {1, 2, 4}, {at::kCUDA});

  auto params = torch_tensorrt::core::conversion::get_named_params(g->inputs(), {});
  auto jit_results = torch_tensorrt::tests::util::RunGraph(g, params, {in1});

  params = torch_tensorrt::core::conversion::get_named_params(g->inputs(), {});
  auto trt_results = torch_tensorrt::tests::util::RunGraphEngine(g, params, {in1});

  ASSERT_TRUE(torch_tensorrt::tests::util::almostEqual(jit_results[0], trt_results[0].reshape_as(jit_results[0]), 2e-6));
}
