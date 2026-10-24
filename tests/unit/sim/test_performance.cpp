#include "core/model.hpp"
#include "core/types.hpp"
#include "sim/simulator.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

#include "unit/core/test_helpers.hpp"

namespace fiovivo {
namespace {

TEST(PerformanceTest, FiveHundredComponentsAndEightHundredWires) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  const ComponentId supply =
      diagram.addComponent(sheet_id, test::makeSupplyAc1(diagram, "X1", {0.0, 0.0})).value();
  const PinId live = test::pinId(diagram, supply, "L");
  const PinId neutral = test::pinId(diagram, supply, "N");

  for (int index = 0; index < 200; ++index) {
    Component coil = test::makeCoil(
        diagram, "KM" + std::to_string(index + 1), {100.0, static_cast<double>(index) * 35.0});
    const ComponentId id = diagram.addComponent(sheet_id, std::move(coil)).value();
    ASSERT_TRUE(diagram.createWire(sheet_id, {supply, live}, {id, test::pinId(diagram, id, "A1")}));
    ASSERT_TRUE(
        diagram.createWire(sheet_id, {supply, neutral}, {id, test::pinId(diagram, id, "A2")}));
  }
  for (int index = 0; index < 200; ++index) {
    Component lamp = test::makeLamp(
        diagram, "H" + std::to_string(index + 1), {400.0, static_cast<double>(index) * 35.0});
    const ComponentId id = diagram.addComponent(sheet_id, std::move(lamp)).value();
    ASSERT_TRUE(diagram.createWire(sheet_id, {supply, live}, {id, test::pinId(diagram, id, "A1")}));
    ASSERT_TRUE(
        diagram.createWire(sheet_id, {supply, neutral}, {id, test::pinId(diagram, id, "A2")}));
  }
  for (int index = 0; index < 99; ++index) {
    Component label = test::makeComponent(diagram,
                                          std::string(component_type::kLabelBox),
                                          "T" + std::to_string(index + 1),
                                          {800.0, static_cast<double>(index) * 30.0},
                                          {},
                                          {60.0, 20.0});
    ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(label)));
  }
  ASSERT_EQ(diagram.componentCount(), 500U);
  ASSERT_EQ(diagram.wireCount(), 800U);

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.step());
  constexpr int kIterations = 50;
  const auto begin = std::chrono::steady_clock::now();
  for (int index = 0; index < kIterations; ++index) {
    ASSERT_TRUE(simulator.step());
  }
  const auto end = std::chrono::steady_clock::now();
  const double total_ms = std::chrono::duration<double, std::milli>(end - begin).count();
  const double per_cycle_ms = total_ms / static_cast<double>(kIterations);
  std::cout << "[PERFORMANCE] 500 components / 800 wires: " << per_cycle_ms
            << " ms per scan cycle\n";
#ifdef NDEBUG
  constexpr double kBudgetMs = 2.0;
#else
  constexpr double kBudgetMs = 50.0;
#endif
  EXPECT_LT(per_cycle_ms, kBudgetMs);
}

} // namespace
} // namespace fiovivo
