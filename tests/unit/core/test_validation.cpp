#include "core/model.hpp"
#include "core/types.hpp"
#include "core/validation.hpp"

#include <gtest/gtest.h>

#include <algorithm>

#include "unit/core/test_helpers.hpp"

namespace fiovivo {
namespace {

[[nodiscard]] bool hasCode(const ValidationReport& report, ProblemCode code) {
  return std::any_of(report.problems.begin(),
                     report.problems.end(),
                     [code](const Problem& problem) { return problem.code == code; });
}

[[nodiscard]] std::size_t
countCode(const ValidationReport& report, ProblemCode code, ProblemSeverity severity) {
  return static_cast<std::size_t>(std::count_if(
      report.problems.begin(), report.problems.end(), [code, severity](const Problem& problem) {
        return problem.code == code && problem.severity == severity;
      }));
}

TEST(ValidationTest, DuplicateTagsAreErrors) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {0.0, 0.0})));
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {100.0, 0.0})));
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(report.hasErrors());
  EXPECT_GE(countCode(report, ProblemCode::DuplicateTag, ProblemSeverity::Error), 1U);
}

TEST(ValidationTest, DuplicateTagsAreValidWhenUnique) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {0.0, 0.0})));
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM2", {100.0, 0.0})));
  const ValidationReport report = validate(diagram);
  EXPECT_EQ(countCode(report, ProblemCode::DuplicateTag, ProblemSeverity::Error), 0U);
}

TEST(ValidationTest, RequiredUnconnectedPinIsWarning) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {0.0, 0.0})));
  const ValidationReport report = validate(diagram);
  EXPECT_GE(countCode(report, ProblemCode::RequiredPinUnconnected, ProblemSeverity::Warning), 1U);
}

TEST(ValidationTest, ConnectedRequiredPinHasNoWarning) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId l = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId n = sheet.findComponent(supply_id)->findPin("N")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, l}, {coil_id, a1}));
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, n}, {coil_id, a2}));
  const ValidationReport report = validate(diagram);
  EXPECT_EQ(countCode(report, ProblemCode::RequiredPinUnconnected, ProblemSeverity::Warning), 0U);
}

TEST(ValidationTest, MatchedContactHasNoWarning) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0}, "KM1");
  Component contact = test::makeAuxNo(diagram, "KM1.1", {100.0, 0.0}, "KM1");
  contact.pins[0].required = false;
  contact.pins[1].required = false;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(contact)));
  const ValidationReport report = validate(diagram);
  EXPECT_FALSE(hasCode(report, ProblemCode::UnmatchedContact));
}

TEST(ValidationTest, UnmatchedContactIsWarning) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component contact = test::makeAuxNo(diagram, "KM1.1", {100.0, 0.0}, "KM7");
  contact.pins[0].required = false;
  contact.pins[1].required = false;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(contact)));
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::UnmatchedContact));
}

TEST(ValidationTest, CoilWithoutContactIsInfo) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  coil.pins[0].required = false;
  coil.pins[1].required = false;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::CoilWithoutContact));
}

TEST(ValidationTest, MissingSupplyPathIsWarningAndCompletePathIsClean) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId l = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, l}, {coil_id, a1}));
  ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::MissingSupplyPath));

  const PinId n = sheet.findComponent(supply_id)->findPin("N")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, n}, {coil_id, a2}));
  report = validate(diagram);
  EXPECT_FALSE(hasCode(report, ProblemCode::MissingSupplyPath));
}

TEST(ValidationTest, FloatingWireIsError) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  Wire wire;
  wire.id = diagram.ids().newWireId();
  wire.from = {coil_id, PinId{999}};
  wire.to = {ComponentId{999}, PinId{1000}};
  wire.route = {{0.0, 0.0}, {10.0, 10.0}};
  diagram.sheets.front().wires.push_back(wire);
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(report.hasErrors());
  EXPECT_TRUE(hasCode(report, ProblemCode::FloatingWire));
}

TEST(ValidationTest, OverlappingComponentsAreWarning) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  Component coil2 = test::makeCoil(diagram, "KM2", {5.0, 5.0});
  coil.pins[0].required = false;
  coil.pins[1].required = false;
  coil2.pins[0].required = false;
  coil2.pins[1].required = false;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil2)));
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::ComponentOverlap));
}

TEST(ValidationTest, CoilHasSupplyPathHelper) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  EXPECT_FALSE(coilHasSupplyPath(diagram, *sheet.findComponent(coil_id)));
  const PinId l = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId n = sheet.findComponent(supply_id)->findPin("N")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, l}, {coil_id, a1}));
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, n}, {coil_id, a2}));
  EXPECT_TRUE(coilHasSupplyPath(diagram, *diagram.findComponent(coil_id)));
}

TEST(ValidationTest, CorruptWiresAndPinBackReferencesAreReported) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  coil.pins[0].required = false;
  coil.pins[1].required = false;
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  Sheet& sheet = diagram.sheets.front();
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;

  Wire self_loop;
  self_loop.id = WireId{900};
  self_loop.from = {coil_id, a1};
  self_loop.to = {coil_id, a2};
  self_loop.allow_self_loop = false;
  self_loop.route = {{0.0, 0.0}, {10.0, 10.0}};
  sheet.wires.push_back(self_loop);
  sheet.findComponent(coil_id)->findPin(a1)->wires.push_back(self_loop.id);
  sheet.findComponent(coil_id)->findPin(a2)->wires.push_back(self_loop.id);
  ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::SelfLoopNotAllowed));

  sheet.wires.clear();
  sheet.findComponent(coil_id)->findPin(a1)->wires.clear();
  sheet.findComponent(coil_id)->findPin(a2)->wires.clear();
  sheet.findComponent(coil_id)->findPin(a1)->wires.push_back(WireId{901});
  report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::InvalidPinConnection));

  Wire same_pin;
  same_pin.id = WireId{902};
  same_pin.from = {coil_id, a1};
  same_pin.to = {coil_id, a1};
  same_pin.route = {{0.0, 0.0}, {1.0, 0.0}};
  sheet.wires.push_back(same_pin);
  sheet.findComponent(coil_id)->findPin(a1)->wires.push_back(same_pin.id);
  report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::InvalidWireEndpoint));
}

TEST(ValidationTest, CoilWithMissingOrSamePotentialPinsIsReported) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {200.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  Component cp = coil;
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  Component wrong = cp;
  wrong.id = diagram.ids().newComponentId();
  wrong.tag = "KM2";
  wrong.pins.clear();
  const ComponentId wrong_id = diagram.addComponent(sheet_id, std::move(wrong)).value();
  (void)wrong_id;

  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId live = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, live}, {coil_id, a1}));
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, live}, {coil_id, a2}));
  const ValidationReport report = validate(diagram);
  EXPECT_GE(countCode(report, ProblemCode::MissingSupplyPath, ProblemSeverity::Warning), 1U);
}

TEST(ValidationTest, DuplicateAndEmptyComponentDataAreReported) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  coil.pins[0].required = false;
  coil.pins[1].required = false;
  ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(coil)));
  Component copy = diagram.sheets.front().components.front();
  copy.id = ComponentId{700};
  copy.tag.clear();
  copy.type.clear();
  diagram.sheets.front().components.push_back(copy);
  const ValidationReport report = validate(diagram);
  EXPECT_TRUE(hasCode(report, ProblemCode::InvalidComponent));
  EXPECT_TRUE(report.hasErrors());
}

TEST(ValidationTest, ReportCounts) {
  ValidationReport report;
  report.add({ProblemSeverity::Error, ProblemCode::DuplicateTag, "e", {}, {}, {}, {}});
  report.add({ProblemSeverity::Warning, ProblemCode::ComponentOverlap, "w", {}, {}, {}, {}});
  report.add({ProblemSeverity::Info, ProblemCode::CoilWithoutContact, "i", {}, {}, {}, {}});
  EXPECT_EQ(report.errorCount(), 1U);
  EXPECT_EQ(report.warningCount(), 1U);
  EXPECT_EQ(report.infoCount(), 1U);
  EXPECT_TRUE(report.hasErrors());
  EXPECT_TRUE(report.hasWarnings());
}

} // namespace
} // namespace fiovivo
