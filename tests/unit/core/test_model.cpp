#include "core/model.hpp"
#include "core/types.hpp"

#include <gtest/gtest.h>

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

  const auto duplicate = diagram.createWire(sheet_id, {coil_id, a1}, {coil_id, a2}, {}, {}, true);
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

TEST(ModelTest, SheetLookupBoundsAndEndpointNames) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component supply = test::makeSupplyAc1(diagram, "X1", {10.0, 20.0});
  Component coil = test::makeCoil(diagram, "KM1", {100.0, 20.0});
  const ComponentId supply_id = diagram.addComponent(sheet_id, std::move(supply)).value();
  const ComponentId coil_id = diagram.addComponent(sheet_id, std::move(coil)).value();
  const Sheet* sheet = diagram.findSheet(sheet_id);
  ASSERT_NE(sheet, nullptr);
  EXPECT_TRUE(sheet->hasComponent(coil_id));
  EXPECT_FALSE(sheet->hasComponent(ComponentId{999}));
  const PinId live = test::pinId(diagram, supply_id, "L");
  const PinId a1 = test::pinId(diagram, coil_id, "A1");
  const auto wire = diagram.createWire(sheet_id, {supply_id, live}, {coil_id, a1});
  ASSERT_TRUE(wire);
  EXPECT_TRUE(sheet->hasWire(wire.value()));
  EXPECT_FALSE(sheet->hasWire(WireId{999}));
  EXPECT_EQ(sheet->wiresAtPin({coil_id, a1}).size(), 1U);
  EXPECT_EQ(endpointName(*sheet, {supply_id, live}), "X1:L");
  EXPECT_EQ(endpointName(*sheet, {ComponentId{999}, PinId{999}}), "?");
  EXPECT_GT(sheet->bounds().width, 0.0);
  EXPECT_NE(diagram.findSheetForComponent(coil_id), nullptr);
  EXPECT_NE(diagram.findPin({coil_id, a1}), nullptr);
  EXPECT_NE(diagram.findWire(wire.value()), nullptr);
  EXPECT_EQ(diagram.findSheetForComponent(ComponentId{999}), nullptr);
  EXPECT_EQ(diagram.findPin({ComponentId{999}, PinId{999}}), nullptr);
  EXPECT_EQ(diagram.findWire(WireId{999}), nullptr);

  const Component* component = diagram.findComponent(coil_id);
  ASSERT_NE(component, nullptr);
  EXPECT_NE(component->pinPosition("A1"), std::nullopt);
  EXPECT_EQ(component->pinPosition("missing"), std::nullopt);
  EXPECT_TRUE(component->hasRole(PinRole::CoilInput));
  EXPECT_FALSE(component->hasRole(PinRole::Earth));

  EXPECT_TRUE(diagram.removeWire(wire.value()));
  EXPECT_FALSE(diagram.removeWire(wire.value()));
  EXPECT_TRUE(sheet->findPin({coil_id, a1})->wires.empty());
  EXPECT_TRUE(diagram.removeSheet(sheet_id));
  EXPECT_FALSE(diagram.removeSheet(sheet_id));
}

TEST(ModelTest, RejectsInvalidComponentsSheetsAndAnnotations) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component no_tag = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  no_tag.tag.clear();
  EXPECT_FALSE(diagram.addComponent(sheet_id, no_tag));
  Component no_type = test::makeCoil(diagram, "KM2", {0.0, 0.0});
  no_type.type.clear();
  EXPECT_FALSE(diagram.addComponent(sheet_id, no_type));
  EXPECT_FALSE(diagram.addComponent(SheetId{999}, test::makeCoil(diagram, "KM3", {0.0, 0.0})));

  Sheet duplicate_sheet = test::makeSheet(diagram);
  duplicate_sheet.id = sheet_id;
  EXPECT_FALSE(diagram.addSheet(std::move(duplicate_sheet)));
  Sheet empty_name;
  empty_name.id = diagram.ids().newSheetId();
  EXPECT_FALSE(diagram.addSheet(std::move(empty_name)));

  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  annotation.text = "one";
  ASSERT_TRUE(diagram.addAnnotation(sheet_id, annotation));
  EXPECT_FALSE(diagram.addAnnotation(sheet_id, annotation));
  EXPECT_FALSE(diagram.addAnnotation(SheetId{999}, annotation));
}

TEST(ModelTest, SheetWireAndAnnotationErrorPaths) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component coil1 = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  Component coil2 = test::makeCoil(diagram, "KM2", {100.0, 0.0});
  const ComponentId first = diagram.addComponent(sheet_id, std::move(coil1)).value();
  const ComponentId second = diagram.addComponent(sheet_id, std::move(coil2)).value();
  Sheet* sheet = diagram.findSheet(sheet_id);
  const PinId a1 = test::pinId(diagram, first, "A1");
  const PinId a2 = test::pinId(diagram, first, "A2");
  const PinId b1 = test::pinId(diagram, second, "A1");
  const PinId b2 = test::pinId(diagram, second, "A2");

  Wire self_loop;
  self_loop.id = diagram.ids().newWireId();
  self_loop.from = {first, a1};
  self_loop.to = {first, a2};
  EXPECT_EQ(sheet->addWire(self_loop).error().code, ErrorCode::SelfLoopNotAllowed);

  Wire same_pin;
  same_pin.id = diagram.ids().newWireId();
  same_pin.from = {first, a1};
  same_pin.to = {first, a1};
  EXPECT_EQ(sheet->addWire(same_pin).error().code, ErrorCode::SamePin);

  Wire missing_pin;
  missing_pin.id = diagram.ids().newWireId();
  missing_pin.from = {first, PinId{999}};
  missing_pin.to = {second, b1};
  EXPECT_EQ(sheet->addWire(missing_pin).error().code, ErrorCode::InvalidReference);

  Wire valid;
  valid.id = diagram.ids().newWireId();
  valid.from = {first, a1};
  valid.to = {second, b1};
  ASSERT_TRUE(sheet->addWire(valid));
  EXPECT_EQ(sheet->addWire(valid).error().code, ErrorCode::DuplicateId);

  Wire duplicate_pair;
  duplicate_pair.id = diagram.ids().newWireId();
  duplicate_pair.from = {second, b1};
  duplicate_pair.to = {first, a1};
  EXPECT_EQ(sheet->addWire(duplicate_pair).error().code, ErrorCode::DuplicateWire);

  // A self-loop is accepted only with the explicit flag.
  Wire allowed_self_loop;
  allowed_self_loop.id = diagram.ids().newWireId();
  allowed_self_loop.from = {first, a1};
  allowed_self_loop.to = {first, a2};
  allowed_self_loop.allow_self_loop = true;
  EXPECT_TRUE(sheet->addWire(allowed_self_loop));
  (void)b2;

  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  ASSERT_TRUE(sheet->addAnnotation(annotation));
  EXPECT_TRUE(sheet->removeAnnotation(annotation.id));
  EXPECT_FALSE(sheet->removeAnnotation(annotation.id));
}

TEST(ModelTest, RebuildsAllocatorAfterExplicitIds) {
  Diagram diagram;
  const SheetId sheet_id = diagram.addSheet(test::makeSheet(diagram)).value();
  Component component = test::makeCoil(diagram, "KM1", {0.0, 0.0});
  component.id = ComponentId{500};
  const ComponentId high = diagram.addComponent(sheet_id, std::move(component)).value();
  EXPECT_EQ(high, ComponentId{500});
  EXPECT_GE(diagram.ids().nextComponentValue(), 501U);
  // Lower the allocator and rebuild it from the data.
  const auto lowered = diagram.ids().newComponentId();
  EXPECT_GT(lowered.value(), 500U);
  diagram.rebuildIdAllocator();
  EXPECT_GT(diagram.ids().newComponentId().value(), 500U);
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
