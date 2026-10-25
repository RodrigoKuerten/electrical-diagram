#include "library/symbol_definition.hpp"

#include "core/model.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <utility>

namespace fiovivo {
namespace {

using nlohmann::json;

[[nodiscard]] Result<Vec2> readSize(const json& value, const std::string& context) {
  if (!value.is_array() || value.size() != 2 || !value[0].is_number() || !value[1].is_number()) {
    return makeError(ErrorCode::ParseError, "Symbol size must be [width, height]", context);
  }
  return Vec2{value[0].get<double>(), value[1].get<double>()};
}

[[nodiscard]] Result<Vec2> readOffset(const json& value, const std::string& context) {
  if (!value.is_object() || !value.contains("x") || !value.contains("y") ||
      !value["x"].is_number() || !value["y"].is_number()) {
    return makeError(ErrorCode::ParseError, "Pin offset must have numeric x and y", context);
  }
  return Vec2{value["x"].get<double>(), value["y"].get<double>()};
}

[[nodiscard]] Result<ParameterValue> readParameterValue(const json& value,
                                                        const std::string& context) {
  if (value.is_boolean()) {
    return ParameterValue{value.get<bool>()};
  }
  if (value.is_number_integer()) {
    return ParameterValue{value.get<std::int64_t>()};
  }
  if (value.is_number_float()) {
    return ParameterValue{value.get<double>()};
  }
  if (value.is_string()) {
    return ParameterValue{value.get<std::string>()};
  }
  return makeError(ErrorCode::ParseError, "Unsupported default parameter value", context);
}

[[nodiscard]] Result<ParameterDefinition> readParameterDefinition(const json& value,
                                                                  const std::string& context) {
  if (!value.is_object() || !value.contains("key") || !value["key"].is_string()) {
    return makeError(ErrorCode::ParseError, "Parameter definition needs a key", context);
  }
  ParameterDefinition definition;
  definition.key = value["key"].get<std::string>();
  const std::string type = value.value("type", std::string("string"));
  if (type == "bool") {
    definition.kind = ParameterKind::Boolean;
  } else if (type == "int") {
    definition.kind = ParameterKind::Integer;
  } else if (type == "number") {
    definition.kind = ParameterKind::Number;
  } else if (type == "string") {
    definition.kind = ParameterKind::String;
  } else if (type == "enum") {
    definition.kind = ParameterKind::Enum;
  } else {
    return makeError(
        ErrorCode::ParseError, "Unknown parameter type: " + type, context + "/" + definition.key);
  }
  definition.label = value.value("label", definition.key);
  definition.label_pt = value.value("label_pt", std::string{});
  definition.unit = value.value("unit", std::string{});
  if (value.contains("options") && value["options"].is_array()) {
    for (const json& option : value["options"]) {
      if (option.is_string()) {
        definition.options.push_back(option.get<std::string>());
      }
    }
  }
  if (value.contains("min") && value["min"].is_number()) {
    definition.minimum = value["min"].get<double>();
  }
  if (value.contains("max") && value["max"].is_number()) {
    definition.maximum = value["max"].get<double>();
  }
  if (definition.kind == ParameterKind::Enum && definition.options.empty()) {
    return makeError(
        ErrorCode::ParseError, "Enum parameter has no options", context + "/" + definition.key);
  }
  return definition;
}

[[nodiscard]] Result<SymbolDefinition> loadManifest(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return makeError(ErrorCode::IoError, "Could not open symbol manifest", path.string());
  }
  std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  const json manifest = json::parse(text, nullptr, false);
  if (manifest.is_discarded() || !manifest.is_object()) {
    return makeError(ErrorCode::ParseError, "Invalid symbol manifest JSON", path.string());
  }
  const std::string context = path.string();
  if (!manifest.contains("type") || !manifest["type"].is_string()) {
    return makeError(ErrorCode::ParseError, "Symbol manifest has no type", context);
  }
  SymbolDefinition definition;
  definition.type = manifest["type"].get<std::string>();
  definition.symbol_version = manifest.value("symbol_version", 0);
  if (definition.symbol_version != 1) {
    return makeError(ErrorCode::UnsupportedVersion, "Unsupported symbol version", definition.type);
  }
  definition.display_name = manifest.value("display_name", definition.type);
  definition.display_name_pt = manifest.value("display_name_pt", std::string{});
  definition.category = manifest.value("category", std::string("other"));
  if (!manifest.contains("size")) {
    return makeError(ErrorCode::ParseError, "Symbol manifest has no size", definition.type);
  }
  const auto size = readSize(manifest["size"], definition.type);
  if (!size) {
    return size.error();
  }
  definition.size = size.value();

  if (!manifest.contains("pins") || !manifest["pins"].is_array()) {
    return makeError(ErrorCode::ParseError, "Symbol manifest has no pins array", definition.type);
  }
  std::set<std::string> pin_names;
  for (const json& pin_value : manifest["pins"]) {
    if (!pin_value.is_object() || !pin_value.contains("name") || !pin_value["name"].is_string()) {
      return makeError(ErrorCode::ParseError, "Malformed pin definition", definition.type);
    }
    PinDefinition pin;
    pin.name = pin_value["name"].get<std::string>();
    if (!pin_names.insert(pin.name).second) {
      return makeError(ErrorCode::ParseError, "Duplicate pin name", pin.name);
    }
    const std::string role = pin_value.value("role", std::string("terminal"));
    const auto parsed_role = pinRoleFromString(role);
    if (!parsed_role.has_value()) {
      return makeError(ErrorCode::ParseError, "Unknown pin role: " + role, pin.name);
    }
    pin.role = *parsed_role;
    const auto offset = readOffset(pin_value, definition.type + "/" + pin.name);
    if (!offset) {
      return offset.error();
    }
    pin.offset = offset.value();
    pin.required = pin_value.value("required", false);
    definition.pins.push_back(std::move(pin));
  }

  if (manifest.contains("default_parameters") && manifest["default_parameters"].is_object()) {
    for (auto iterator = manifest["default_parameters"].begin();
         iterator != manifest["default_parameters"].end();
         ++iterator) {
      const auto value = readParameterValue(iterator.value(), definition.type);
      if (!value) {
        return value.error();
      }
      definition.default_parameters[iterator.key()] = value.value();
    }
  }
  if (manifest.contains("parameter_schema") && manifest["parameter_schema"].is_array()) {
    for (const json& parameter : manifest["parameter_schema"]) {
      const auto parsed = readParameterDefinition(parameter, definition.type);
      if (!parsed) {
        return parsed.error();
      }
      definition.parameter_definitions.push_back(parsed.value());
    }
  }
  if (manifest.contains("states") && manifest["states"].is_object()) {
    for (auto iterator = manifest["states"].begin(); iterator != manifest["states"].end();
         ++iterator) {
      if (!iterator.value().is_string()) {
        return makeError(
            ErrorCode::ParseError, "Symbol state filename must be a string", definition.type);
      }
      definition.states[iterator.key()] = iterator.value().get<std::string>();
    }
  }
  if (manifest.contains("element_ids") && manifest["element_ids"].is_object()) {
    for (auto iterator = manifest["element_ids"].begin(); iterator != manifest["element_ids"].end();
         ++iterator) {
      if (!iterator.value().is_array()) {
        continue;
      }
      std::vector<std::string> ids;
      for (const json& id : iterator.value()) {
        if (id.is_string()) {
          ids.push_back(id.get<std::string>());
        }
      }
      definition.element_ids[iterator.key()] = std::move(ids);
    }
  }
  return definition;
}

} // namespace

const PinDefinition* SymbolDefinition::findPin(std::string_view name) const {
  const auto iterator = std::find_if(
      pins.begin(), pins.end(), [name](const PinDefinition& pin) { return pin.name == name; });
  return iterator == pins.end() ? nullptr : &*iterator;
}

const ParameterDefinition* SymbolDefinition::findParameter(std::string_view key) const {
  const auto iterator =
      std::find_if(parameter_definitions.begin(),
                   parameter_definitions.end(),
                   [key](const ParameterDefinition& parameter) { return parameter.key == key; });
  return iterator == parameter_definitions.end() ? nullptr : &*iterator;
}

Result<void> SymbolLibrary::loadFromDirectory(const std::filesystem::path& directory) {
  std::error_code error;
  if (!std::filesystem::is_directory(directory, error)) {
    return makeError(ErrorCode::IoError, "Symbol directory does not exist", directory.string());
  }
  std::vector<std::filesystem::path> manifests;
  for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
    if (entry.is_regular_file() && entry.path().extension() == ".json") {
      manifests.push_back(entry.path());
    }
  }
  std::sort(manifests.begin(), manifests.end());
  std::map<std::string, SymbolDefinition> loaded;
  for (const std::filesystem::path& path : manifests) {
    const auto definition = loadManifest(path);
    if (!definition) {
      return definition.error();
    }
    if (loaded.contains(definition.value().type)) {
      return makeError(ErrorCode::DuplicateId, "Duplicate symbol type", definition.value().type);
    }
    const std::filesystem::path base_svg = directory / (definition.value().type + ".svg");
    if (!std::filesystem::is_regular_file(base_svg, error)) {
      return makeError(ErrorCode::IoError, "Symbol SVG is missing", base_svg.string());
    }
    for (const auto& [state, filename] : definition.value().states) {
      (void)state;
      const std::filesystem::path state_svg = directory / filename;
      if (!std::filesystem::is_regular_file(state_svg, error)) {
        return makeError(ErrorCode::IoError, "Symbol state SVG is missing", state_svg.string());
      }
    }
    loaded[definition.value().type] = definition.value();
  }
  if (loaded.empty()) {
    return makeError(ErrorCode::NotFound, "No symbol manifests found", directory.string());
  }
  definitions_ = std::move(loaded);
  directory_ = directory;
  return {};
}

const SymbolDefinition* SymbolLibrary::find(std::string_view type) const {
  const auto iterator = definitions_.find(std::string(type));
  return iterator == definitions_.end() ? nullptr : &iterator->second;
}

std::vector<std::string> SymbolLibrary::categories() const {
  std::set<std::string> categories;
  for (const auto& [type, definition] : definitions_) {
    (void)type;
    categories.insert(definition.category);
  }
  return {categories.begin(), categories.end()};
}

std::string SymbolLibrary::defaultTagBase(std::string_view type) {
  using namespace component_type;
  const auto starts = [type](std::string_view prefix) {
    return type.size() >= prefix.size() && type.substr(0, prefix.size()) == prefix;
  };
  if (type == kSupplyAc3 || type == kSupplyAc1 || type == kSupplyDc || starts("supply_")) {
    return "X1";
  }
  if (type == kFuse || type == kThermalRelay || starts("fuse") || starts("contact_thermal")) {
    return "F1";
  }
  if (type == kBreaker || starts("breaker") || starts("rcd_") || type == "motor_breaker_3p") {
    return "Q1";
  }
  if (type == kContactorCoil || starts("contactor_coil") || type == "contactor_combo") {
    return "KM1";
  }
  if (starts("relay_coil") || type == "relay_combo" || type == "teleruptor") {
    return "K1";
  }
  if (starts("solenoid_valve")) {
    return "YV1";
  }
  if (type == "horn") {
    return "H1";
  }
  if (type == kContactorNo || type == kContactorAuxNo || type == kContactorAuxNc ||
      starts("contactor_aux") || starts("relay_contact") || starts("contact_") ||
      starts("contactor_")) {
    return "KM1.1";
  }
  if (type == kPushbuttonNo || type == kPushbuttonNc || starts("pushbutton_")) {
    return "S1";
  }
  if (type == kEmergencyStop || starts("mushroom_")) {
    return "S0";
  }
  if (type == kSelectorSwitch || starts("selector_")) {
    return "SEL1";
  }
  if (type == kTimerOn || type == kTimerOff || starts("timer_")) {
    return "TM1";
  }
  if (starts("flasher") || starts("counter") || starts("time_switch")) {
    return "KT1";
  }
  if (type == kPilotLamp || starts("pilot_lamp") || starts("lamp_") || starts("led")) {
    return "H1";
  }
  if (type == kMotor3ph || starts("motor_") || type == "linear_actuator") {
    return "M1";
  }
  if (type == kTerminal || starts("terminal") || starts("cable") || type == "junction") {
    return "XT1";
  }
  if (type == kLabelBox) {
    return "TXT1";
  }
  if (starts("logic_") || starts("display_")) {
    return "U1";
  }
  if (starts("plc_") || starts("arduino")) {
    return "PLC1";
  }
  if (starts("pneumatic_valve")) {
    return "YV1";
  }
  if (starts("pneumatic_") || starts("valve_")) {
    return "M1";
  }
  if (starts("meter_")) {
    return "P1";
  }
  if (starts("grafcet_")) {
    return "E1";
  }
  if (starts("sensor_") || starts("limit_") || starts("pressure_") || starts("vacuum_") ||
      starts("thermostat_") || starts("float_") || starts("reed_")) {
    return "S1";
  }
  if (starts("switch_")) {
    return "S1";
  }
  return "X1";
}

Component instantiateSymbol(const SymbolDefinition& definition,
                            Diagram& diagram,
                            Vec2 position,
                            std::string tag) {
  Component component;
  component.id = diagram.ids().newComponentId();
  component.type = definition.type;
  component.tag = std::move(tag);
  component.position = position;
  component.size = definition.size;
  component.parameters = definition.default_parameters;
  for (const PinDefinition& definition_pin : definition.pins) {
    Pin pin;
    pin.id = diagram.ids().newPinId();
    pin.name = definition_pin.name;
    pin.role = definition_pin.role;
    pin.offset = definition_pin.offset;
    pin.required = definition_pin.required;
    component.pins.push_back(std::move(pin));
  }
  return component;
}

} // namespace fiovivo
