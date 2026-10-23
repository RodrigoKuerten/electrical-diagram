#pragma once

#include "core/model.hpp"
#include "core/types.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fiovivo::test {

struct PinSpec {
  std::string name;
  PinRole role;
  Vec2 offset;
  bool required = false;
};

inline Component makeComponent(Diagram& diagram,
                               std::string type,
                               std::string tag,
                               Vec2 position,
                               std::vector<PinSpec> pins,
                               Vec2 size = {40.0, 40.0}) {
  Component component;
  component.id = diagram.ids().newComponentId();
  component.type = std::move(type);
  component.tag = std::move(tag);
  component.position = position;
  component.size = size;
  for (const PinSpec& spec : pins) {
    Pin pin;
    pin.id = diagram.ids().newPinId();
    pin.name = spec.name;
    pin.role = spec.role;
    pin.offset = spec.offset;
    pin.required = spec.required;
    component.pins.push_back(std::move(pin));
  }
  return component;
}

inline Sheet makeSheet(Diagram& diagram, std::string name = "Sheet 1") {
  Sheet sheet;
  sheet.id = diagram.ids().newSheetId();
  sheet.name = std::move(name);
  return sheet;
}

inline Component makeSupplyAc1(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kSupplyAc1),
                       std::move(tag),
                       position,
                       {{"L", PinRole::SupplyIn, {20.0, 0.0}, true},
                        {"N", PinRole::Neutral, {20.0, 20.0}, true},
                        {"PE", PinRole::Earth, {20.0, 30.0}, false}},
                       {40.0, 30.0});
}

inline Component
makeCoil(Diagram& diagram, std::string tag, Vec2 position, std::string group = {}) {
  Component component = makeComponent(diagram,
                                      std::string(component_type::kContactorCoil),
                                      std::move(tag),
                                      position,
                                      {{"A1", PinRole::CoilInput, {10.0, 0.0}, true},
                                       {"A2", PinRole::CoilOutput, {10.0, 30.0}, true}},
                                      {40.0, 30.0});
  if (!group.empty()) {
    component.parameters["label_group"] = group;
  }
  return component;
}

inline Component
makeAuxNo(Diagram& diagram, std::string tag, Vec2 position, std::string group = {}) {
  Component component = makeComponent(diagram,
                                      std::string(component_type::kContactorAuxNo),
                                      std::move(tag),
                                      position,
                                      {{"13", PinRole::ContactCommon, {15.0, 0.0}, true},
                                       {"14", PinRole::ContactNo, {15.0, 30.0}, true}},
                                      {30.0, 30.0});
  if (!group.empty()) {
    component.parameters["label_group"] = group;
  }
  return component;
}

inline Component makeLamp(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kPilotLamp),
                       std::move(tag),
                       position,
                       {{"A1", PinRole::CoilInput, {10.0, 0.0}, true},
                        {"A2", PinRole::CoilOutput, {10.0, 20.0}, true}},
                       {30.0, 20.0});
}

inline Component
makeAuxNc(Diagram& diagram, std::string tag, Vec2 position, std::string group = {}) {
  Component component = makeComponent(diagram,
                                      std::string(component_type::kContactorAuxNc),
                                      std::move(tag),
                                      position,
                                      {{"21", PinRole::ContactCommon, {15.0, 0.0}, true},
                                       {"22", PinRole::ContactNc, {15.0, 30.0}, true}},
                                      {30.0, 30.0});
  if (!group.empty()) {
    component.parameters["label_group"] = group;
  }
  return component;
}

inline Component
makePushbutton(Diagram& diagram, bool normally_open, std::string tag, Vec2 position) {
  const std::string type = normally_open ? std::string(component_type::kPushbuttonNo)
                                         : std::string(component_type::kPushbuttonNc);
  const std::string common = normally_open ? "13" : "21";
  const std::string contact = normally_open ? "14" : "22";
  Component component = makeComponent(
      diagram,
      type,
      std::move(tag),
      position,
      {{common, PinRole::ContactCommon, {15.0, 0.0}, true},
       {contact, normally_open ? PinRole::ContactNo : PinRole::ContactNc, {15.0, 30.0}, true}},
      {30.0, 30.0});
  component.parameters["mode"] = std::string("momentary");
  return component;
}

inline Component makeEmergencyStop(Diagram& diagram, std::string tag, Vec2 position) {
  Component component = makeComponent(diagram,
                                      std::string(component_type::kEmergencyStop),
                                      std::move(tag),
                                      position,
                                      {{"1", PinRole::ContactCommon, {17.0, 0.0}, true},
                                       {"2", PinRole::ContactNc, {17.0, 44.0}, true}},
                                      {34.0, 44.0});
  component.parameters["latching"] = true;
  return component;
}

inline Component makeFuse(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kFuse),
                       std::move(tag),
                       position,
                       {{"1", PinRole::ContactCommon, {10.0, 0.0}, true},
                        {"2", PinRole::ContactCommon, {10.0, 40.0}, true}},
                       {20.0, 40.0});
}

inline Component makeBreaker(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kBreaker),
                       std::move(tag),
                       position,
                       {{"1", PinRole::ContactCommon, {12.0, 0.0}, true},
                        {"2", PinRole::ContactCommon, {12.0, 44.0}, true}},
                       {24.0, 44.0});
}

inline Component
makeTimer(Diagram& diagram, bool on_delay, std::string tag, Vec2 position, std::int64_t delay_ms) {
  Component component = makeComponent(diagram,
                                      on_delay ? std::string(component_type::kTimerOn)
                                               : std::string(component_type::kTimerOff),
                                      std::move(tag),
                                      position,
                                      {{"A1", PinRole::CoilInput, {0.0, 15.0}, true},
                                       {"A2", PinRole::CoilOutput, {0.0, 35.0}, true},
                                       {"15", PinRole::ContactCommon, {70.0, 15.0}, true},
                                       {"18", PinRole::ContactNo, {70.0, 35.0}, true}},
                                      {70.0, 50.0});
  component.parameters["delay_ms"] = delay_ms;
  return component;
}

inline Component makeThermalRelay(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kThermalRelay),
                       std::move(tag),
                       position,
                       {{"95", PinRole::ContactCommon, {0.0, 15.0}, true},
                        {"96", PinRole::ContactNc, {60.0, 15.0}, true},
                        {"97", PinRole::ContactCommon, {0.0, 45.0}, true},
                        {"98", PinRole::ContactNo, {60.0, 45.0}, true}},
                       {60.0, 60.0});
}

inline Component makeSelector(Diagram& diagram, std::string tag, Vec2 position, int positions = 2) {
  Component component = makeComponent(diagram,
                                      std::string(component_type::kSelectorSwitch),
                                      std::move(tag),
                                      position,
                                      {{"1", PinRole::ContactCommon, {0.0, 25.0}, true},
                                       {"2", PinRole::ContactNo, {60.0, 10.0}, true},
                                       {"3", PinRole::ContactNo, {60.0, 25.0}, true},
                                       {"4", PinRole::ContactNo, {60.0, 40.0}, true}},
                                      {60.0, 50.0});
  component.parameters["positions"] = static_cast<std::int64_t>(positions);
  component.parameters["position"] = std::int64_t{1};
  return component;
}

inline Component makeSupplyAc3(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kSupplyAc3),
                       std::move(tag),
                       position,
                       {{"L1", PinRole::SupplyIn, {40.0, 10.0}, true},
                        {"L2", PinRole::SupplyIn, {40.0, 22.0}, true},
                        {"L3", PinRole::SupplyIn, {40.0, 34.0}, true},
                        {"N", PinRole::Neutral, {40.0, 46.0}, false},
                        {"PE", PinRole::Earth, {40.0, 56.0}, false}},
                       {40.0, 60.0});
}

inline Component makeMotor(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram,
                       std::string(component_type::kMotor3ph),
                       std::move(tag),
                       position,
                       {{"U1", PinRole::SupplyIn, {0.0, 14.0}, true},
                        {"V1", PinRole::SupplyIn, {0.0, 28.0}, true},
                        {"W1", PinRole::SupplyIn, {0.0, 42.0}, true}},
                       {56.0, 56.0});
}

inline PinId pinId(const Diagram& diagram, ComponentId component_id, std::string_view name) {
  const Component* component = diagram.findComponent(component_id);
  if (component == nullptr) {
    return {};
  }
  const Pin* pin = component->findPin(name);
  return pin == nullptr ? PinId{} : pin->id;
}

inline WireId connect(Diagram& diagram,
                      SheetId sheet_id,
                      ComponentId from,
                      std::string_view from_pin,
                      ComponentId to,
                      std::string_view to_pin,
                      bool allow_self_loop = false) {
  const PinId first = pinId(diagram, from, from_pin);
  const PinId second = pinId(diagram, to, to_pin);
  if (!first.valid() || !second.valid()) {
    return {};
  }
  const auto result =
      diagram.createWire(sheet_id, {from, first}, {to, second}, {}, {}, allow_self_loop);
  return result ? result.value() : WireId{};
}

} // namespace fiovivo::test
