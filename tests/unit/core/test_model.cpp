#include <gtest/gtest.h>

#include "core/model.hpp"
#include "core/types.hpp"
#include "unit/core/test_helpers.hpp"

namespace fiovivo {
namespace {

TEST(ModelTest, AddsSheetComponentAndPin) {
  Diagram diagram;
  Sheet sheet = test::makeSheet(diagram);
  ASSERT_TRUE(diagram.addSheet(std::move(sheet)));
  Component coil = test::makeCoil(diagram, "KM1", {10.0, 10.0});
  const auto coil_id = diagram.addComponent(diagram.sheets.front().id, std::move(coil));
  ASSERT_TRUE(coil_id);
  EXPECT_EQ(diagram.componentCount(), 1U);
  ASSERT_NE(diagram.findComponent(coil_id.value()), nullptr);
  EXPECT_EQ(diagram.findComponent(coil_id.value())->tag, "KM1");
  EXPECT_NE(diagram.findComponentByTag("KM1"), nullptr);
  EXPECT_EQ(diagram.findComponentByTag("missing"), nullptr);
  const Component& stored = *diagram.findComponent(coil_id.value());
  EXPECT_EQ(stored.pinCount(), 2U);
  ASSERT_NE(stored.findPin("A1"), nullptr);
  EXPECT_NE(stored.findPin("A2"), nullptr);
  EXPECT_EQ(stored.findPin("A1")->role, PinRole::CoilInput);
}

TEST(ModelTest, RejectsDuplicateComponentId) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component component = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  ASSERT_TRUE(diagram.addComponent(sheet_id, component));
  const auto result = diagram.addComponent(sheet_id, component);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::DuplicateId);
}

TEST(ModelTest, RejectsDuplicatePinIdsAndNames) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component component = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  component.pins[1].name = "A1";
  const auto result = diagram.addComponent(sheet_id, std::move(component));
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::InvalidArgument);

  Component duplicate_id = test::makeCoil(diagram, "KM2", {0.0, 0.0});
  duplicate_id.pins[1].id = duplicate_id.pins[0].id;
  const auto result2 = diagram.addComponent(sheet_id, std::move(duplicate_id));
  ASSERT_FALSE(result2);
  EXPECT_EQ(result2.error().code, ErrorCode::DuplicateId);
}

TEST(ModelTest, CreatesWireAndTracksPins) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const Pin* supply_l = sheet.findComponent(supply_id)->findPin("L");
  const Pin* coil_a1 = sheet.findComponent(coil_id)->findPin("A1");
  ASSERT_NE(supply_l, nullptr);
  ASSERT_NE(coil_a1, nullptr);
  const auto wire_id =
      diagram.createWire(sheet_id, {supply_id, supply_l->id}, {coil_id, coil_a1->id});
  ASSERT_TRUE(wire_id);
  const Wire* wire = diagram.findWire(wire_id.value());
  ASSERT_NE(wire, nullptr);
  EXPECT_GE(wire->route.size(), 2U);
  EXPECT_EQ(diagram.wireCount(), 1U);
  EXPECT_EQ(sheet.findPin({supply_id, supply_l->id})->wires.size(), 1U);
  EXPECT_EQ(sheet.findPin({coil_id, coil_a1->id})->wires.size(), 1U);
  EXPECT_EQ(sheet.wiresAtPin({coil_id, coil_a1->id}).size(), 1U);
}

TEST(ModelTest, AllowsMultipleWiresAtOnePin) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  Component lamp = test::makeLamp(diagram, "H1", {200.0, 0.0});
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const ComponentId lamp_id = diagram.addComponent(sheet_id, std::move(lamp)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId supply_l = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId coil_a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId lamp_a1 = sheet.findComponent(lamp_id)->findPin("A1")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, supply_l}, {coil_id, coil_a1}));
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, supply_l}, {lamp_id, lamp_a1}));
  EXPECT_EQ(diagram.findWire(diagram.sheets.front().wires.front().id) != nullptr, true);
  EXPECT_EQ(sheet.findPin({supply_id, supply_l})->wires.size(), 2U);
}

TEST(ModelTest, RejectsInvalidWires) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  const PinId a2 = sheet.findComponent(coil_id)->findPin("A2")->id;

  const auto same_pin = diagram.createWire(sheet_id, {coil_id, a1}, {coil_id, a1});
  ASSERT_FALSE(same_pin);
  EXPECT_EQ(same_pin.error().code, ErrorCode::SamePin);

  const auto self_loop = diagram.createWire(sheet_id, {coil_id, a1}, {coil_id, a2});
  ASSERT_FALSE(self_loop);
  EXPECT_EQ(self_loop.error().code, ErrorCode::SelfLoopNotAllowed);

  const auto allowed = diagram.createWire(sheet_id, {coil_id, a1}, {coil_id, a2}, {}, {}, true);
  ASSERT_TRUE(allowed);

  const auto duplicate =
      diagram.createWire(sheet_id, {coil_id, a1}, {coil_id, a2}, {}, {}, true);
  ASSERT_FALSE(duplicate);
  EXPECT_EQ(duplicate.error().code, ErrorCode::DuplicateWire);

  const auto missing = diagram.createWire(sheet_id, {coil_id, a1}, {ComponentId{999}, a1});
  ASSERT_FALSE(missing);
  EXPECT_EQ(missing.error().code, ErrorCode::InvalidReference);
}

TEST(ModelTest, RemoveComponentRemovesAttachedWires) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {0.0, 0.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 0.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet& sheet = *diagram.findSheet(sheet_id);
  const PinId l = sheet.findComponent(supply_id)->findPin("L")->id;
  const PinId a1 = sheet.findComponent(coil_id)->findPin("A1")->id;
  ASSERT_TRUE(diagram.createWire(sheet_id, {supply_id, l}, {coil_id, a1}));
  EXPECT_TRUE(diagram.removeComponent(coil_id));
  EXPECT_EQ(diagram.wireCount(), 0U);
  EXPECT_EQ(diagram.componentCount(), 1U);
  EXPECT_TRUE(diagram.sheets.front().findPin({supply_id, l})->wires.empty());
  EXPECT_FALSE(diagram.removeComponent(coil_id));
}

TEST(ModelTest, UniqueTagsAndBaseName) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {0.0, 0.0})));
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM2", {100.0, 0.0})));
  EXPECT_TRUE(diagram.isTagUsed("KM1"));
  EXPECT_FALSE(diagram.isTagUsed("KM3"));
  EXPECT_EQ(diagram.uniqueTag("KM1"), "KM3");
  EXPECT_EQ(diagram.uniqueTag("S1"), "S1");
  EXPECT_EQ(tagBaseName("KM1"), "KM1");
  EXPECT_EQ(tagBaseName("KM1.2"), "KM1");
  EXPECT_EQ(tagBaseName("KM1:13-14"), "KM1");
  EXPECT_EQ(tagBaseName("KM1#copy"), "KM1");
}

TEST(ModelTest, LabelGroupUsesParameterThenBaseTag) {
  Diagram diagram;
  Component coil = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  EXPECT_EQ(coil.labelGroup(), "KM1");
  coil.tag = "KM1.2";
  EXPECT_EQ(coil.labelGroup(), "KM1");
  coil.parameters["label_group"] = std::string("KM9");
  EXPECT_EQ(coil.labelGroup(), "KM9");
}

TEST(ModelTest, PinPositionRespectsRotation) {
  Diagram diagram;
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 100.0});
  coil.size = {40.0, 30.0};
  const Pin* a1 = coil.findPin("A1");
  ASSERT_NE(a1, nullptr);
  EXPECT_EQ(coil.pinPosition(*a1), (Vec2{110.0, 100.0}));
  coil.rotation = Rotation::Deg90;
  const Vec2 rotated = coil.pinPosition(*a1);
  // Center is (120, 115); A1 at (110,100) becomes (135,105).
  EXPECT_NEAR(rotated.x, 135.0, 1e-12);
  EXPECT_NEAR(rotated.y, 105.0, 1e-12);
}

TEST(ModelTest, BoundsRespectRotation) {
  Diagram diagram;
  Component coil = test::makeCoil(diagram, "KM1", {10.0, 20.0});
  coil.size = {40.0, 20.0};
  const Rect normal = coil.boundingBox();
  EXPECT_EQ(normal, (Rect{10.0, 20.0, 40.0, 20.0}));
  coil.rotation = Rotation::Deg90;
  const Rect rotated = coil.boundingBox();
  EXPECT_NEAR(rotated.width, 20.0, 1e-12);
  EXPECT_NEAR(rotated.height, 40.0, 1e-12);
}

TEST(ModelTest, IdAllocatorObservesLoadedIds) {
  IdAllocator allocator;
  EXPECT_EQ(allocator.newComponentId(), ComponentId{1});
  allocator.observe(ComponentId{10});
  EXPECT_EQ(allocator.newComponentId(), ComponentId{11});
  allocator.observe(PinId{4});
  EXPECT_EQ(allocator.newPinId(), PinId{5});
  allocator.observe(WireId{7});
  EXPECT_EQ(allocator.newWireId(), WireId{8});
}

TEST(ModelTest, RemovesSheetAndAnnotation) {
  Diagram diagram;
  Sheet sheet = test::makeSheet(diagram);
  const SheetId sheet_id = sheet.id;
  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  annotation.text = "hello";
  ASSERT_TRUE(sheet.addAnnotation(annotation));
  ASSERT_TRUE(diagram.addSheet(std::move(sheet)));
  ASSERT_TRUE(diagram.findSheet(sheet_id)->removeAnnotation(annotation.id));
  EXPECT_TRUE(diagram.findSheet(sheet_id)->annotations.empty());
  EXPECT_TRUE(diagram.removeSheet(sheet_id));
  EXPECT_TRUE(diagram.sheets.empty());
}

TEST(ModelTest, EqualDiagramRoundTripsByValue) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  ASSERT_TRUE(diagram.addComponent(sheet_id, test::makeCoil(diagram, "KM1", {0.0, 0.0})));
  Diagram copy = diagram;
  EXPECT_EQ(copy, diagram);
  copy.sheets.front().components.front().tag = "KM2";
  EXPECT_NE(copy, diagram);
}

} // namespace
} // namespace fiovivo
