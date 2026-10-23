#include "core/model.hpp"
#include "core/types.hpp"
#include "sim/simulator.hpp"

#include <gtest/gtest.h>

#include "unit/core/test_helpers.hpp"

namespace fiovivo::sim {
namespace {

struct TimerBench {
  Diagram diagram;
  SheetId sheet_id;
  ComponentId supply;
  ComponentId start;
  ComponentId timer;
  ComponentId lamp;

  explicit TimerBench(bool on_delay, std::int64_t delay_ms = 30) {
    sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
    supply = diagram.addComponent(sheet_id, test::makeSupplyAc1(diagram, "X1", {0.0, 0.0})).value();
    start = diagram.addComponent(sheet_id, test::makePushbutton(diagram, true, "S1", {100.0, 0.0}))
                .value();
    timer = diagram
                .addComponent(sheet_id,
                              test::makeTimer(diagram, on_delay, "TM1", {200.0, 0.0}, delay_ms))
                .value();
    lamp = diagram.addComponent(sheet_id, test::makeLamp(diagram, "H1", {400.0, 0.0})).value();
    EXPECT_TRUE(test::connect(diagram, sheet_id, supply, "L", start, "13").valid());
    EXPECT_TRUE(test::connect(diagram, sheet_id, start, "14", timer, "A1").valid());
    EXPECT_TRUE(test::connect(diagram, sheet_id, timer, "A2", supply, "N", false).valid());
    EXPECT_TRUE(test::connect(diagram, sheet_id, supply, "L", timer, "15").valid());
    EXPECT_TRUE(test::connect(diagram, sheet_id, timer, "18", lamp, "A1").valid());
    EXPECT_TRUE(test::connect(diagram, sheet_id, supply, "N", lamp, "A2").valid());
  }
};

TEST(TimerTest, TonEnergizesAtExactDelay) {
  TimerBench bench(true, 30);
  Simulator simulator(bench.diagram);

  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).energized);
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 10);
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
  EXPECT_FALSE(simulator.state(bench.lamp).energized);

  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 20);
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);

  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 30);
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  EXPECT_TRUE(simulator.state(bench.lamp).energized);
  EXPECT_TRUE(simulator.state(bench.timer).closed);
  EXPECT_EQ(simulator.cycleCount(), 3U);
  EXPECT_EQ(simulator.simulatedTimeMs(), 30);
}

TEST(TimerTest, TonResetsWhenInputDropsEvenMidCount) {
  TimerBench bench(true, 30);
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 20);

  ASSERT_TRUE(simulator.setInput(bench.start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 0);
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
  EXPECT_FALSE(simulator.state(bench.lamp).energized);

  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 10);
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
}

TEST(TimerTest, TonResetsAfterOutputWasActive) {
  TimerBench bench(true, 20);
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  ASSERT_TRUE(simulator.setInput(bench.start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 0);
  EXPECT_FALSE(simulator.state(bench.lamp).energized);
}

TEST(TimerTest, TofHoldsOutputDuringOffDelay) {
  TimerBench bench(false, 30);
  Simulator simulator(bench.diagram);

  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).energized);
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  EXPECT_TRUE(simulator.state(bench.lamp).energized);

  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.setInput(bench.start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(bench.timer).energized);
  EXPECT_TRUE(simulator.state(bench.timer).timer_output) << "off-delay must hold the output";
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 10);

  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 20);

  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 30);
  EXPECT_FALSE(simulator.state(bench.lamp).energized);
}

TEST(TimerTest, TofCancelsOffDelayWhenInputReturns) {
  TimerBench bench(false, 30);
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.setInput(bench.start, false));
  ASSERT_TRUE(simulator.step());
  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 20);

  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  EXPECT_EQ(simulator.state(bench.timer).timer_elapsed_ms, 0);
}

TEST(TimerTest, ZeroDelaySwitchesImmediately) {
  TimerBench bench(true, 0);
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
  EXPECT_TRUE(simulator.state(bench.lamp).energized);
}

TEST(TimerTest, TimeBaseSecondsIsConverted) {
  TimerBench bench(true, 1);
  // Replace the delay parameter with 1 second.
  Component& timer = *bench.diagram.findComponent(bench.timer);
  timer.parameters["time_base"] = std::string("s");
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  for (int index = 0; index < 99; ++index) {
    ASSERT_TRUE(simulator.step());
  }
  EXPECT_FALSE(simulator.state(bench.timer).timer_output);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
}

TEST(TimerTest, NegativeDelayIsClampedToZero) {
  TimerBench bench(true, -50);
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(bench.start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(bench.timer).timer_output);
}

} // namespace
} // namespace fiovivo::sim
