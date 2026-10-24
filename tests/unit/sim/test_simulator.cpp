#include "core/model.hpp"
#include "core/types.hpp"
#include "sim/simulator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <string>

#include "unit/core/test_helpers.hpp"

namespace fiovivo::sim {
namespace {

struct Bench {
  Diagram diagram;
  SheetId sheet_id;

  Bench() { sheet_id = diagram.addSheet(test::makeSheet(diagram)).value(); }

  ComponentId add(Component component) {
    return diagram.addComponent(sheet_id, std::move(component)).value();
  }

  WireId
  wire(ComponentId from, std::string_view from_pin, ComponentId to, std::string_view to_pin) {
    return test::connect(diagram, sheet_id, from, from_pin, to, to_pin);
  }
};

TEST(SimulatorTest, EnergizesLampAcrossSupplyAndReturn) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  EXPECT_FALSE(simulator.state(lamp).energized);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp).energized);
  EXPECT_EQ(simulator.liveWires().size(), 2U);
  EXPECT_EQ(simulator.simulatedTimeMs(), 10);
}

TEST(SimulatorTest, DoesNotEnergizeLampOnSamePhase) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(lamp).energized);
}

TEST(SimulatorTest, ContactorContactPowersLampAfterCoil) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "KM1", {100.0, 0.0}));
  const ComponentId contact =
      bench.add(test::makeAuxNo(bench.diagram, "KM1.1", {200.0, 0.0}, "KM1"));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "L", contact, "13").valid());
  ASSERT_TRUE(bench.wire(contact, "14", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  EXPECT_TRUE(simulator.state(contact).closed);
  EXPECT_TRUE(simulator.state(lamp).energized);
}

TEST(SimulatorTest, SelfHoldingKeepsCoilAfterButtonRelease) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId start =
      bench.add(test::makePushbutton(bench.diagram, true, "S1", {100.0, 0.0}));
  const ComponentId seal = bench.add(test::makeAuxNo(bench.diagram, "KM1.1", {200.0, 0.0}, "KM1"));
  const ComponentId stop =
      bench.add(test::makePushbutton(bench.diagram, false, "S2", {300.0, 0.0}));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "KM1", {400.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {500.0, 0.0}));

  ASSERT_TRUE(bench.wire(supply, "L", start, "13").valid());
  ASSERT_TRUE(bench.wire(supply, "L", seal, "13").valid());
  ASSERT_TRUE(bench.wire(start, "14", stop, "21").valid());
  ASSERT_TRUE(bench.wire(seal, "14", stop, "21").valid());
  ASSERT_TRUE(bench.wire(stop, "22", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(stop, "22", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  EXPECT_TRUE(simulator.state(seal).closed);
  EXPECT_TRUE(simulator.state(lamp).energized);

  ASSERT_TRUE(simulator.setInput(start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized) << "the seal-in contact must hold the coil";
  EXPECT_TRUE(simulator.state(lamp).energized);

  ASSERT_TRUE(simulator.setInput(stop, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  EXPECT_FALSE(simulator.state(seal).closed);
  EXPECT_FALSE(simulator.state(lamp).energized);

  ASSERT_TRUE(simulator.setInput(stop, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized) << "coil must not restart without start";
}

TEST(SimulatorTest, InterlockLetsFirstCoilWinDeterministically) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId start1 =
      bench.add(test::makePushbutton(bench.diagram, true, "S1", {100.0, 0.0}));
  const ComponentId start2 =
      bench.add(test::makePushbutton(bench.diagram, true, "S2", {100.0, 80.0}));
  const ComponentId interlock1 =
      bench.add(test::makeAuxNc(bench.diagram, "K1.1", {200.0, 0.0}, "K1"));
  const ComponentId interlock2 =
      bench.add(test::makeAuxNc(bench.diagram, "K2.1", {200.0, 80.0}, "K2"));
  const ComponentId coil1 = bench.add(test::makeCoil(bench.diagram, "K1", {300.0, 0.0}));
  const ComponentId coil2 = bench.add(test::makeCoil(bench.diagram, "K2", {300.0, 80.0}));

  ASSERT_TRUE(bench.wire(supply, "L", start1, "13").valid());
  ASSERT_TRUE(bench.wire(start1, "14", interlock2, "21").valid());
  ASSERT_TRUE(bench.wire(interlock2, "22", coil1, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil1, "A2").valid());

  ASSERT_TRUE(bench.wire(supply, "L", start2, "13").valid());
  ASSERT_TRUE(bench.wire(start2, "14", interlock1, "21").valid());
  ASSERT_TRUE(bench.wire(interlock1, "22", coil2, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil2, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(start1, true));
  ASSERT_TRUE(simulator.setInput(start2, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil1).energized);
  EXPECT_FALSE(simulator.state(coil2).energized);
  EXPECT_FALSE(simulator.state(interlock1).closed);
  EXPECT_TRUE(simulator.state(interlock2).closed);
}

TEST(SimulatorTest, EmergencyStopLatchesUntilExplicitReset) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId stop = bench.add(test::makeEmergencyStop(bench.diagram, "S0", {100.0, 0.0}));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "KM1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", stop, "1").valid());
  ASSERT_TRUE(bench.wire(stop, "2", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);

  ASSERT_TRUE(simulator.setInput(stop, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  for (int index = 0; index < 5; ++index) {
    ASSERT_TRUE(simulator.step());
    EXPECT_FALSE(simulator.state(coil).energized);
  }

  ASSERT_TRUE(simulator.setInput(stop, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
}

TEST(SimulatorTest, ThermalRelaySwitchesBothContactSets) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId thermal = bench.add(test::makeThermalRelay(bench.diagram, "F1", {100.0, 0.0}));
  const ComponentId lamp_nc = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  const ComponentId lamp_no = bench.add(test::makeLamp(bench.diagram, "H2", {500.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", thermal, "95").valid());
  ASSERT_TRUE(bench.wire(thermal, "96", lamp_nc, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp_nc, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "L", thermal, "97").valid());
  ASSERT_TRUE(bench.wire(thermal, "98", lamp_no, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp_no, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp_nc).energized);
  EXPECT_FALSE(simulator.state(lamp_no).energized);

  ASSERT_TRUE(simulator.setTripped(thermal, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(lamp_nc).energized);
  EXPECT_TRUE(simulator.state(lamp_no).energized);

  ASSERT_TRUE(simulator.setTripped(thermal, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp_nc).energized);
  EXPECT_FALSE(simulator.state(lamp_no).energized);
}

TEST(SimulatorTest, FuseAndBreakerOpenWhenTripped) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId fuse = bench.add(test::makeFuse(bench.diagram, "F1", {100.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", fuse, "1").valid());
  ASSERT_TRUE(bench.wire(fuse, "2", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp).energized);
  ASSERT_TRUE(simulator.setTripped(fuse, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(lamp).energized);
  ASSERT_TRUE(simulator.setTripped(fuse, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp).energized);
}

TEST(SimulatorTest, SelectorChangesSelectedContact) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId selector = bench.add(test::makeSelector(bench.diagram, "S1", {100.0, 0.0}));
  const ComponentId lamp1 = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  const ComponentId lamp2 = bench.add(test::makeLamp(bench.diagram, "H2", {300.0, 100.0}));
  ASSERT_TRUE(bench.wire(supply, "L", selector, "1").valid());
  ASSERT_TRUE(bench.wire(selector, "2", lamp1, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp1, "A2").valid());
  ASSERT_TRUE(bench.wire(selector, "3", lamp2, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp2, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp1).energized);
  EXPECT_FALSE(simulator.state(lamp2).energized);

  ASSERT_TRUE(simulator.setSelectorPosition(selector, 2));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(lamp1).energized);
  EXPECT_TRUE(simulator.state(lamp2).energized);

  const auto invalid = simulator.setSelectorPosition(selector, 3);
  EXPECT_FALSE(invalid);
}

TEST(SimulatorTest, ThreePhaseMotorRunsWithTwoPhases) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc3(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId motor = bench.add(test::makeMotor(bench.diagram, "M1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L1", motor, "U1").valid());
  ASSERT_TRUE(bench.wire(supply, "L2", motor, "V1").valid());
  ASSERT_TRUE(bench.wire(supply, "L3", motor, "W1").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(motor).energized);
  EXPECT_TRUE(simulator.hasFault() == false);
}

TEST(SimulatorTest, OscillatingSelfReferenceIsDetected) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId contact = bench.add(test::makeAuxNc(bench.diagram, "K1.1", {100.0, 0.0}, "K1"));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "K1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", contact, "21").valid());
  ASSERT_TRUE(bench.wire(contact, "22", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());

  Simulator simulator(bench.diagram, SimulationOptions{10, 16, 20000});
  const auto result = simulator.step();
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::Oscillation);
  EXPECT_TRUE(simulator.hasFault());
  const auto second = simulator.step();
  ASSERT_FALSE(second);
  EXPECT_EQ(second.error().code, ErrorCode::Oscillation);
}

TEST(SimulatorTest, ResetRestoresInitialStateAndTime) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_EQ(simulator.simulatedTimeMs(), 10);
  ASSERT_TRUE(simulator.reset());
  EXPECT_EQ(simulator.simulatedTimeMs(), 0);
  EXPECT_EQ(simulator.cycleCount(), 0U);
  EXPECT_TRUE(simulator.log().empty());
  EXPECT_FALSE(simulator.state(lamp).energized);
}

TEST(SimulatorTest, DeterministicReplayProducesSameSnapshotAndLog) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId start =
      bench.add(test::makePushbutton(bench.diagram, true, "S1", {100.0, 0.0}));
  const ComponentId seal = bench.add(test::makeAuxNo(bench.diagram, "KM1.1", {200.0, 0.0}, "KM1"));
  const ComponentId stop =
      bench.add(test::makePushbutton(bench.diagram, false, "S2", {300.0, 0.0}));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "KM1", {400.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", start, "13").valid());
  ASSERT_TRUE(bench.wire(supply, "L", seal, "13").valid());
  ASSERT_TRUE(bench.wire(start, "14", stop, "21").valid());
  ASSERT_TRUE(bench.wire(seal, "14", stop, "21").valid());
  ASSERT_TRUE(bench.wire(stop, "22", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());

  Simulator first(bench.diagram);
  Simulator second(bench.diagram);
  for (Simulator* simulator : {&first, &second}) {
    ASSERT_TRUE(simulator->setInput(start, true));
    for (int index = 0; index < 3; ++index) {
      ASSERT_TRUE(simulator->step());
    }
    ASSERT_TRUE(simulator->setInput(start, false));
    for (int index = 0; index < 4; ++index) {
      ASSERT_TRUE(simulator->step());
    }
    ASSERT_TRUE(simulator->setInput(stop, true));
    for (int index = 0; index < 2; ++index) {
      ASSERT_TRUE(simulator->step());
    }
  }
  EXPECT_EQ(first.snapshot(), second.snapshot());
  EXPECT_EQ(first.log(), second.log());
}

TEST(SimulatorTest, SnapshotRestoreContinuesIdentically) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId start =
      bench.add(test::makePushbutton(bench.diagram, true, "S1", {100.0, 0.0}));
  const ComponentId coil = bench.add(test::makeCoil(bench.diagram, "KM1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", start, "13").valid());
  ASSERT_TRUE(bench.wire(start, "14", coil, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil, "A2").valid());

  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  const SimulationSnapshot checkpoint = simulator.snapshot();
  for (int index = 0; index < 3; ++index) {
    ASSERT_TRUE(simulator.step());
  }
  ASSERT_TRUE(simulator.restore(checkpoint));
  EXPECT_EQ(simulator.snapshot(), checkpoint);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
}

TEST(SimulatorTest, RejectsUnknownSnapshotComponents) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());
  Simulator simulator(bench.diagram);
  SimulationSnapshot snapshot = simulator.snapshot();
  snapshot.states[ComponentId{999}] = ComponentState{};
  const auto result = simulator.restore(snapshot);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::InvalidReference);
}

TEST(SimulatorTest, BreakerTripAndMotorSinglePhase) {
  Bench breaker_bench;
  const ComponentId supply =
      breaker_bench.add(test::makeSupplyAc1(breaker_bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId breaker =
      breaker_bench.add(test::makeBreaker(breaker_bench.diagram, "Q1", {100.0, 0.0}));
  const ComponentId lamp =
      breaker_bench.add(test::makeLamp(breaker_bench.diagram, "H1", {300.0, 0.0}));
  ASSERT_TRUE(breaker_bench.wire(supply, "L", breaker, "1").valid());
  ASSERT_TRUE(breaker_bench.wire(breaker, "2", lamp, "A1").valid());
  ASSERT_TRUE(breaker_bench.wire(supply, "N", lamp, "A2").valid());
  Simulator breaker_sim(breaker_bench.diagram);
  ASSERT_TRUE(breaker_sim.step());
  EXPECT_TRUE(breaker_sim.state(lamp).energized);
  ASSERT_TRUE(breaker_sim.setTripped(breaker, true));
  ASSERT_TRUE(breaker_sim.step());
  EXPECT_FALSE(breaker_sim.state(lamp).energized);

  Bench motor_bench;
  const ComponentId phases =
      motor_bench.add(test::makeSupplyAc3(motor_bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId motor =
      motor_bench.add(test::makeMotor(motor_bench.diagram, "M1", {300.0, 0.0}));
  ASSERT_TRUE(motor_bench.wire(phases, "L1", motor, "U1").valid());
  Simulator motor_sim(motor_bench.diagram);
  ASSERT_TRUE(motor_sim.step());
  EXPECT_FALSE(motor_sim.state(motor).energized);
}

TEST(SimulatorTest, MultipleCoilsShareAGroupForContacts) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId coil1 = bench.add(test::makeCoil(bench.diagram, "KM1", {100.0, 0.0}, "SHARED"));
  const ComponentId coil2 =
      bench.add(test::makeCoil(bench.diagram, "KM2", {100.0, 100.0}, "SHARED"));
  const ComponentId contact =
      bench.add(test::makeAuxNo(bench.diagram, "KM1.1", {200.0, 0.0}, "SHARED"));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", coil1, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", coil1, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "L", contact, "13").valid());
  ASSERT_TRUE(bench.wire(contact, "14", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil1).energized);
  EXPECT_FALSE(simulator.state(coil2).energized);
  EXPECT_TRUE(simulator.state(contact).closed);
  EXPECT_TRUE(simulator.state(lamp).energized);
}

TEST(SimulatorTest, ThreePositionSelector) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId selector = bench.add(test::makeSelector(bench.diagram, "S1", {100.0, 0.0}, 3));
  const ComponentId lamp1 = bench.add(test::makeLamp(bench.diagram, "H1", {300.0, 0.0}));
  const ComponentId lamp2 = bench.add(test::makeLamp(bench.diagram, "H2", {300.0, 100.0}));
  const ComponentId lamp3 = bench.add(test::makeLamp(bench.diagram, "H3", {300.0, 200.0}));
  ASSERT_TRUE(bench.wire(supply, "L", selector, "1").valid());
  ASSERT_TRUE(bench.wire(selector, "2", lamp1, "A1").valid());
  ASSERT_TRUE(bench.wire(selector, "3", lamp2, "A1").valid());
  ASSERT_TRUE(bench.wire(selector, "4", lamp3, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp1, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp2, "A2").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp3, "A2").valid());
  Simulator simulator(bench.diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp1).energized);
  ASSERT_TRUE(simulator.setSelectorPosition(selector, 2));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp2).energized);
  EXPECT_FALSE(simulator.state(lamp1).energized);
  ASSERT_TRUE(simulator.setSelectorPosition(selector, 3));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(lamp3).energized);
  EXPECT_FALSE(simulator.state(lamp2).energized);
}

TEST(SimulatorTest, SnapshotValidationAndFaultRecovery) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());
  Simulator simulator(bench.diagram);
  SimulationSnapshot snapshot = simulator.snapshot();
  snapshot.time_ms = -1;
  EXPECT_FALSE(simulator.restore(snapshot));
  snapshot.time_ms = 0;
  snapshot.states.erase(lamp);
  EXPECT_FALSE(simulator.restore(snapshot));
  snapshot = simulator.snapshot();
  snapshot.inputs[ComponentId{999}] = DigitalInputState{};
  EXPECT_FALSE(simulator.restore(snapshot));
  EXPECT_FALSE(simulator.restore(SimulationSnapshot{}));
}

TEST(SimulatorTest, InputSettersValidateComponentTypes) {
  Bench bench;
  const ComponentId supply = bench.add(test::makeSupplyAc1(bench.diagram, "X1", {0.0, 0.0}));
  const ComponentId lamp = bench.add(test::makeLamp(bench.diagram, "H1", {100.0, 0.0}));
  ASSERT_TRUE(bench.wire(supply, "L", lamp, "A1").valid());
  ASSERT_TRUE(bench.wire(supply, "N", lamp, "A2").valid());
  Simulator simulator(bench.diagram);
  EXPECT_FALSE(simulator.setInput(lamp, true));
  EXPECT_FALSE(simulator.setTripped(lamp, true));
  EXPECT_FALSE(simulator.setSelectorPosition(lamp, 2));
  EXPECT_FALSE(simulator.setInput(ComponentId{999}, true));
}

} // namespace
} // namespace fiovivo::sim
