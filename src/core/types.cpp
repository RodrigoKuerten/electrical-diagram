#include "core/types.hpp"

#include <array>

namespace fiovivo {

std::string_view toString(PinRole role) {
  switch (role) {
  case PinRole::Terminal:
    return "terminal";
  case PinRole::CoilInput:
    return "coil_input";
  case PinRole::CoilOutput:
    return "coil_output";
  case PinRole::ContactCommon:
    return "contact_common";
  case PinRole::ContactNo:
    return "contact_no";
  case PinRole::ContactNc:
    return "contact_nc";
  case PinRole::SupplyIn:
    return "supply_in";
  case PinRole::SupplyOut:
    return "supply_out";
  case PinRole::Neutral:
    return "neutral";
  case PinRole::Earth:
    return "earth";
  case PinRole::SignalIn:
    return "signal_in";
  case PinRole::SignalOut:
    return "signal_out";
  }
  return "terminal";
}

std::optional<PinRole> pinRoleFromString(std::string_view value) {
  static const std::array table{
      std::pair{"terminal", PinRole::Terminal},
      std::pair{"coil_input", PinRole::CoilInput},
      std::pair{"coil_output", PinRole::CoilOutput},
      std::pair{"contact_common", PinRole::ContactCommon},
      std::pair{"contact_no", PinRole::ContactNo},
      std::pair{"contact_nc", PinRole::ContactNc},
      std::pair{"supply_in", PinRole::SupplyIn},
      std::pair{"supply_out", PinRole::SupplyOut},
      std::pair{"neutral", PinRole::Neutral},
      std::pair{"earth", PinRole::Earth},
      std::pair{"signal_in", PinRole::SignalIn},
      std::pair{"signal_out", PinRole::SignalOut},
  };
  for (const auto& [name, role] : table) {
    if (name == value) {
      return role;
    }
  }
  return std::nullopt;
}

bool isSourceRole(PinRole role) {
  return role == PinRole::SupplyIn || role == PinRole::SupplyOut || role == PinRole::Neutral;
}

std::string_view toString(PaperSize size) {
  switch (size) {
  case PaperSize::A4:
    return "A4";
  case PaperSize::A3:
    return "A3";
  case PaperSize::A2:
    return "A2";
  case PaperSize::A1:
    return "A1";
  case PaperSize::Letter:
    return "Letter";
  }
  return "A4";
}

std::string_view toString(Orientation orientation) {
  return orientation == Orientation::Landscape ? "landscape" : "portrait";
}

std::optional<PaperSize> paperSizeFromString(std::string_view value) {
  if (value == "A4") {
    return PaperSize::A4;
  }
  if (value == "A3") {
    return PaperSize::A3;
  }
  if (value == "A2") {
    return PaperSize::A2;
  }
  if (value == "A1") {
    return PaperSize::A1;
  }
  if (value == "Letter") {
    return PaperSize::Letter;
  }
  return std::nullopt;
}

std::optional<Orientation> orientationFromString(std::string_view value) {
  if (value == "landscape") {
    return Orientation::Landscape;
  }
  if (value == "portrait") {
    return Orientation::Portrait;
  }
  return std::nullopt;
}

Vec2 paperDimensions(PaperSize size, Orientation orientation) {
  Vec2 landscape{};
  switch (size) {
  case PaperSize::A4:
    landscape = {297.0, 210.0};
    break;
  case PaperSize::A3:
    landscape = {420.0, 297.0};
    break;
  case PaperSize::A2:
    landscape = {594.0, 420.0};
    break;
  case PaperSize::A1:
    landscape = {841.0, 594.0};
    break;
  case PaperSize::Letter:
    landscape = {279.4, 215.9};
    break;
  }
  if (orientation == Orientation::Portrait) {
    return {landscape.y, landscape.x};
  }
  return landscape;
}

std::string_view toString(WireDash dash) {
  switch (dash) {
  case WireDash::Solid:
    return "solid";
  case WireDash::Dashed:
    return "dashed";
  case WireDash::Dotted:
    return "dotted";
  }
  return "solid";
}

std::optional<WireDash> wireDashFromString(std::string_view value) {
  if (value == "solid") {
    return WireDash::Solid;
  }
  if (value == "dashed") {
    return WireDash::Dashed;
  }
  if (value == "dotted") {
    return WireDash::Dotted;
  }
  return std::nullopt;
}

std::string_view toString(AnnotationKind kind) {
  switch (kind) {
  case AnnotationKind::Text:
    return "text";
  case AnnotationKind::Rectangle:
    return "rectangle";
  case AnnotationKind::Legend:
    return "legend";
  }
  return "text";
}

std::optional<AnnotationKind> annotationKindFromString(std::string_view value) {
  if (value == "text") {
    return AnnotationKind::Text;
  }
  if (value == "rectangle") {
    return AnnotationKind::Rectangle;
  }
  if (value == "legend") {
    return AnnotationKind::Legend;
  }
  return std::nullopt;
}

namespace {
template<typename T>
[[nodiscard]] T parameterAs(const ParameterMap& parameters, std::string_view key, T fallback) {
  const auto iterator = parameters.find(std::string(key));
  if (iterator == parameters.end()) {
    return fallback;
  }
  if (const auto* value = std::get_if<T>(&iterator->second)) {
    return *value;
  }
  return fallback;
}
} // namespace

bool parameterBool(const ParameterMap& parameters, std::string_view key, bool fallback) {
  return parameterAs<bool>(parameters, key, fallback);
}

std::int64_t
parameterInt(const ParameterMap& parameters, std::string_view key, std::int64_t fallback) {
  const auto iterator = parameters.find(std::string(key));
  if (iterator == parameters.end()) {
    return fallback;
  }
  if (const auto* value = std::get_if<std::int64_t>(&iterator->second)) {
    return *value;
  }
  if (const auto* value = std::get_if<double>(&iterator->second)) {
    return static_cast<std::int64_t>(*value);
  }
  return fallback;
}

double parameterNumber(const ParameterMap& parameters, std::string_view key, double fallback) {
  const auto iterator = parameters.find(std::string(key));
  if (iterator == parameters.end()) {
    return fallback;
  }
  if (const auto* value = std::get_if<double>(&iterator->second)) {
    return *value;
  }
  if (const auto* value = std::get_if<std::int64_t>(&iterator->second)) {
    return static_cast<double>(*value);
  }
  return fallback;
}

std::string
parameterString(const ParameterMap& parameters, std::string_view key, std::string fallback) {
  const auto iterator = parameters.find(std::string(key));
  if (iterator == parameters.end()) {
    return fallback;
  }
  if (const auto* value = std::get_if<std::string>(&iterator->second)) {
    return *value;
  }
  return fallback;
}

const ParameterValue* findParameter(const ParameterMap& parameters, std::string_view key) {
  const auto iterator = parameters.find(std::string(key));
  return iterator == parameters.end() ? nullptr : &iterator->second;
}

ComponentCategory categorizeComponent(std::string_view type) {
  using namespace component_type;
  if (type == kSupplyAc3 || type == kSupplyAc1 || type == kSupplyDc) {
    return ComponentCategory::Supply;
  }
  if (type == kFuse || type == kBreaker || type == kThermalRelay) {
    return ComponentCategory::Protection;
  }
  if (type == kContactorCoil || type == kContactorNo || type == kContactorAuxNo ||
      type == kContactorAuxNc) {
    return ComponentCategory::Contactor;
  }
  if (type == kPushbuttonNo || type == kPushbuttonNc || type == kEmergencyStop ||
      type == kSelectorSwitch) {
    return ComponentCategory::Control;
  }
  if (type == kTimerOn || type == kTimerOff) {
    return ComponentCategory::Timer;
  }
  if (type == kPilotLamp) {
    return ComponentCategory::Indication;
  }
  if (type == kMotor3ph) {
    return ComponentCategory::Machine;
  }
  if (type == kTerminal) {
    return ComponentCategory::Terminal;
  }
  if (type == kLabelBox) {
    return ComponentCategory::Annotation;
  }
  return ComponentCategory::Unknown;
}

bool isContactorContact(std::string_view type) {
  using namespace component_type;
  return type == kContactorNo || type == kContactorAuxNo || type == kContactorAuxNc;
}

bool isPushbutton(std::string_view type) {
  using namespace component_type;
  return type == kPushbuttonNo || type == kPushbuttonNc;
}

bool isTimer(std::string_view type) {
  return type == component_type::kTimerOn || type == component_type::kTimerOff;
}

bool isCoil(std::string_view type) {
  using namespace component_type;
  return type == kContactorCoil || type == kTimerOn || type == kTimerOff;
}

bool isSupply(std::string_view type) {
  using namespace component_type;
  return type == kSupplyAc3 || type == kSupplyAc1 || type == kSupplyDc;
}

bool isLoad(std::string_view type) {
  using namespace component_type;
  return type == kContactorCoil || type == kPilotLamp || type == kMotor3ph || type == kTimerOn ||
         type == kTimerOff;
}

bool isUserActuated(std::string_view type) {
  using namespace component_type;
  return type == kPushbuttonNo || type == kPushbuttonNc || type == kEmergencyStop ||
         type == kSelectorSwitch || type == kThermalRelay || type == kFuse || type == kBreaker;
}

} // namespace fiovivo
