#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/model.hpp"
#include "core/types.hpp"

namespace fiovivo::test {

struct PinSpec {
  std::string name;
  PinRole role;
  Vec2 offset;
  bool required = false;
};

inline Component makeComponent(Diagram& diagram, std::string type, std::string tag,
                               Vec2 position, std::vector<PinSpec> pins,
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
  return makeComponent(
      diagram, std::string(component_type::kSupplyAc1), std::move(tag), position,
      {{"L", PinRole::SupplyIn, {20.0, 0.0}, true},
       {"N", PinRole::Neutral, {20.0, 20.0}, true},
       {"PE", PinRole::Earth, {20.0, 30.0}, false}},
      {40.0, 30.0});
}

inline Component makeCoil(Diagram& diagram, std::string tag, Vec2 position,
                          std::string group = {}) {
  Component component = makeComponent(
      diagram, std::string(component_type::kContactorCoil), std::move(tag), position,
      {{"A1", PinRole::CoilInput, {10.0, 0.0}, true},
       {"A2", PinRole::CoilOutput, {10.0, 30.0}, true}},
      {40.0, 30.0});
  if (!group.empty()) {
    component.parameters["label_group"] = group;
  }
  return component;
}

inline Component makeAuxNo(Diagram& diagram, std::string tag, Vec2 position,
                           std::string group = {}) {
  Component component = makeComponent(
      diagram, std::string(component_type::kContactorAuxNo), std::move(tag), position,
      {{"13", PinRole::ContactCommon, {15.0, 0.0}, true},
       {"14", PinRole::ContactNo, {15.0, 30.0}, true}},
      {30.0, 30.0});
  if (!group.empty()) {
    component.parameters["label_group"] = group;
  }
  return component;
}

inline Component makeLamp(Diagram& diagram, std::string tag, Vec2 position) {
  return makeComponent(diagram, std::string(component_type::kPilotLamp), std::move(tag), position,
                       {{"A1", PinRole::CoilInput, {10.0, 0.0}, true},
                        {"A2", PinRole::CoilOutput, {10.0, 20.0}, true}},
                       {30.0, 20.0});
}

} // namespace fiovivo::test
