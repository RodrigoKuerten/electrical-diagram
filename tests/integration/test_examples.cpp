#include "core/model.hpp"
#include "core/types.hpp"
#include "core/validation.hpp"
#include "io/bom.hpp"
#include "io/json_io.hpp"
#include "sim/simulator.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#ifndef FIOVIVO_EXAMPLES_DIR
  #define FIOVIVO_EXAMPLES_DIR "examples"
#endif

#ifndef FIOVIVO_TEST_DATA_DIR
  #define FIOVIVO_TEST_DATA_DIR "tests/data"
#endif

namespace fiovivo {
namespace {

[[nodiscard]] std::filesystem::path examplePath(const std::string& name) {
  return std::filesystem::path(FIOVIVO_EXAMPLES_DIR) / name;
}

[[nodiscard]] Diagram loadExample(const std::string& name) {
  const auto loaded = io::loadFile(examplePath(name));
  EXPECT_TRUE(loaded) << loaded.error().toString();
  return loaded ? loaded.value() : Diagram{};
}

[[nodiscard]] ComponentId tag(const Diagram& diagram, std::string_view name) {
  const Component* component = diagram.findComponentByTag(name);
  EXPECT_NE(component, nullptr) << name;
  return component == nullptr ? ComponentId{} : component->id;
}

TEST(ExampleTest, AllExamplesLoadValidateAndRoundTrip) {
  for (const char* name : {"partida-direta.dcj",
                           "partida-estrela-triangulo.dcj",
                           "sequencia-temporizada.dcj",
                           "intertravamento-dois-motores.dcj",
                           "parada-emergencia.dcj"}) {
    SCOPED_TRACE(name);
    const auto loaded = io::loadFile(examplePath(name));
    ASSERT_TRUE(loaded) << loaded.error().toString();
    const ValidationReport report = validate(loaded.value());
    EXPECT_FALSE(report.hasErrors());
    const auto first = io::toJsonString(loaded.value());
    ASSERT_TRUE(first);
    const auto second_load = io::fromJsonString(first.value());
    ASSERT_TRUE(second_load);
    const auto second = io::toJsonString(second_load.value());
    ASSERT_TRUE(second);
    EXPECT_EQ(first.value(), second.value());
  }
}

TEST(ExampleTest, BomMatchesGoldenFile) {
  const Diagram diagram = loadExample("partida-direta.dcj");
  const auto csv = io::bomToCsv(diagram);
  ASSERT_TRUE(csv);
  const std::filesystem::path golden =
      std::filesystem::path(FIOVIVO_TEST_DATA_DIR) / "golden" / "partida-direta-bom.csv";
  std::ifstream input(golden, std::ios::binary);
  ASSERT_TRUE(input) << golden.string();
  std::ostringstream expected;
  expected << input.rdbuf();
  EXPECT_EQ(csv.value(), expected.str());
}

TEST(ExampleTest, DirectOnStartLatchingAndThermalTrip) {
  Diagram diagram = loadExample("partida-direta.dcj");
  const ComponentId start = tag(diagram, "S1");
  const ComponentId stop = tag(diagram, "S2");
  const ComponentId coil = tag(diagram, "KM1");
  const ComponentId main_contact = tag(diagram, "KM1.1");
  const ComponentId thermal = tag(diagram, "F1");
  const ComponentId motor = tag(diagram, "M1");
  const ComponentId lamp_on = tag(diagram, "H1");
  const ComponentId lamp_off = tag(diagram, "H2");
  const ComponentId lamp_trip = tag(diagram, "H3");

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  EXPECT_FALSE(simulator.state(motor).energized);
  EXPECT_TRUE(simulator.state(lamp_off).energized);
  EXPECT_FALSE(simulator.state(lamp_on).energized);
  EXPECT_FALSE(simulator.state(lamp_trip).energized);

  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  EXPECT_TRUE(simulator.state(main_contact).closed);
  EXPECT_TRUE(simulator.state(motor).energized);
  EXPECT_TRUE(simulator.state(lamp_on).energized);
  EXPECT_FALSE(simulator.state(lamp_off).energized);

  ASSERT_TRUE(simulator.setInput(start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized) << "self-holding must keep the coil on";
  EXPECT_TRUE(simulator.state(motor).energized);

  ASSERT_TRUE(simulator.setInput(stop, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  EXPECT_FALSE(simulator.state(motor).energized);
  EXPECT_FALSE(simulator.state(lamp_on).energized);
  EXPECT_TRUE(simulator.state(lamp_off).energized);

  ASSERT_TRUE(simulator.setInput(stop, false));
  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  ASSERT_TRUE(simulator.setTripped(thermal, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  EXPECT_FALSE(simulator.state(motor).energized);
  EXPECT_TRUE(simulator.state(lamp_trip).energized);
}

TEST(ExampleTest, StarDeltaTransitionsWithoutOverlap) {
  Diagram diagram = loadExample("partida-estrela-triangulo.dcj");
  const ComponentId start = tag(diagram, "S1");
  const ComponentId stop = tag(diagram, "S0");
  const ComponentId main_coil = tag(diagram, "KM1");
  const ComponentId star_coil = tag(diagram, "KM2");
  const ComponentId delta_coil = tag(diagram, "KM3");
  const ComponentId timer = tag(diagram, "TM1");
  const ComponentId motor = tag(diagram, "M1");

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(main_coil).energized);
  EXPECT_TRUE(simulator.state(star_coil).energized);
  EXPECT_FALSE(simulator.state(delta_coil).energized);
  EXPECT_TRUE(simulator.state(motor).energized);
  EXPECT_TRUE(simulator.state(timer).energized);

  bool saw_delta = false;
  for (int cycle = 0; cycle < 320; ++cycle) {
    ASSERT_TRUE(simulator.step());
    const bool star = simulator.state(star_coil).energized;
    const bool delta = simulator.state(delta_coil).energized;
    EXPECT_FALSE(star && delta) << "star and delta closed simultaneously at cycle " << cycle;
    saw_delta = saw_delta || delta;
    EXPECT_EQ(simulator.state(timer).timer_output, delta)
        << "timer output and delta should agree after the delay";
  }
  EXPECT_TRUE(saw_delta);
  EXPECT_FALSE(simulator.state(star_coil).energized);
  EXPECT_TRUE(simulator.state(delta_coil).energized);

  ASSERT_TRUE(simulator.setInput(stop, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(main_coil).energized);
  EXPECT_FALSE(simulator.state(star_coil).energized);
  EXPECT_FALSE(simulator.state(delta_coil).energized);
  EXPECT_FALSE(simulator.state(motor).energized);
}

TEST(ExampleTest, TimerSequenceUsesExactDelays) {
  Diagram diagram = loadExample("sequencia-temporizada.dcj");
  const ComponentId start = tag(diagram, "S1");
  const ComponentId on_timer = tag(diagram, "TM1");
  const ComponentId off_timer = tag(diagram, "TM2");
  const ComponentId lamp_on = tag(diagram, "H1");
  const ComponentId lamp_off = tag(diagram, "H2");

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.setInput(start, true));
  for (int cycle = 0; cycle < 199; ++cycle) {
    ASSERT_TRUE(simulator.step());
  }
  EXPECT_FALSE(simulator.state(lamp_on).energized);
  EXPECT_TRUE(simulator.state(lamp_off).energized);
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(on_timer).timer_output);
  EXPECT_TRUE(simulator.state(lamp_on).energized);

  ASSERT_TRUE(simulator.setInput(start, false));
  for (int cycle = 0; cycle < 299; ++cycle) {
    ASSERT_TRUE(simulator.step());
  }
  EXPECT_TRUE(simulator.state(lamp_off).energized);
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(off_timer).timer_output);
  EXPECT_FALSE(simulator.state(lamp_off).energized);
}

TEST(ExampleTest, TwoMotorInterlockOnlyEnergizesFirst) {
  Diagram diagram = loadExample("intertravamento-dois-motores.dcj");
  const ComponentId start1 = tag(diagram, "S1");
  const ComponentId start2 = tag(diagram, "S2");
  const ComponentId stop1 = tag(diagram, "S3");
  const ComponentId coil1 = tag(diagram, "K1");
  const ComponentId coil2 = tag(diagram, "K2");
  const ComponentId motor1 = tag(diagram, "M1");
  const ComponentId motor2 = tag(diagram, "M2");

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.setInput(start1, true));
  ASSERT_TRUE(simulator.setInput(start2, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil1).energized);
  EXPECT_FALSE(simulator.state(coil2).energized);
  EXPECT_TRUE(simulator.state(motor1).energized);
  EXPECT_FALSE(simulator.state(motor2).energized);

  ASSERT_TRUE(simulator.setInput(start1, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil1).energized);
  EXPECT_FALSE(simulator.state(coil2).energized);

  ASSERT_TRUE(simulator.setInput(start2, false));
  ASSERT_TRUE(simulator.setInput(stop1, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil1).energized);
  EXPECT_FALSE(simulator.state(motor1).energized);

  ASSERT_TRUE(simulator.setInput(start2, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil2).energized);
  EXPECT_TRUE(simulator.state(motor2).energized);
}

TEST(ExampleTest, EmergencyStopLatchBlocksRestartUntilReset) {
  Diagram diagram = loadExample("parada-emergencia.dcj");
  const ComponentId stop = tag(diagram, "S0");
  const ComponentId start = tag(diagram, "S1");
  const ComponentId coil = tag(diagram, "KM1");
  const ComponentId lamp = tag(diagram, "H1");

  sim::Simulator simulator(diagram);
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);

  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  EXPECT_TRUE(simulator.state(lamp).energized);
  ASSERT_TRUE(simulator.setInput(start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);

  ASSERT_TRUE(simulator.setInput(stop, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);
  ASSERT_TRUE(simulator.setInput(start, true));
  ASSERT_TRUE(simulator.step());
  EXPECT_FALSE(simulator.state(coil).energized);

  ASSERT_TRUE(simulator.setInput(stop, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
  ASSERT_TRUE(simulator.setInput(start, false));
  ASSERT_TRUE(simulator.step());
  EXPECT_TRUE(simulator.state(coil).energized);
}

TEST(ExampleTest, DeterministicReplayOfDirectOn) {
  Diagram diagram = loadExample("partida-direta.dcj");
  const ComponentId start = tag(diagram, "S1");
  const ComponentId stop = tag(diagram, "S2");
  const ComponentId thermal = tag(diagram, "F1");

  sim::Simulator first(diagram);
  sim::Simulator second(diagram);
  for (sim::Simulator* simulator : {&first, &second}) {
    ASSERT_TRUE(simulator->setInput(start, true));
    for (int i = 0; i < 5; ++i) {
      ASSERT_TRUE(simulator->step());
    }
    ASSERT_TRUE(simulator->setInput(start, false));
    for (int i = 0; i < 5; ++i) {
      ASSERT_TRUE(simulator->step());
    }
    ASSERT_TRUE(simulator->setTripped(thermal, true));
    ASSERT_TRUE(simulator->step());
    ASSERT_TRUE(simulator->setTripped(thermal, false));
    ASSERT_TRUE(simulator->setInput(stop, true));
    ASSERT_TRUE(simulator->step());
  }
  EXPECT_EQ(first.snapshot(), second.snapshot());
  EXPECT_EQ(first.log(), second.log());
}

} // namespace
} // namespace fiovivo
