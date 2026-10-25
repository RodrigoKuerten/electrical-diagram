#include "ui/ui_helpers.hpp"

#include "library/symbol_definition.hpp"

#include <QCoreApplication>
#include <QLocale>

namespace fiovivo::ui {

namespace {

[[nodiscard]] bool startsWith(std::string_view value, std::string_view prefix) {
  return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] bool isSensorLike(const Component& component) {
  return startsWith(component.type, "sensor_") || startsWith(component.type, "pressure_") ||
         startsWith(component.type, "vacuum_") || startsWith(component.type, "thermostat_") ||
         startsWith(component.type, "float_") || startsWith(component.type, "reed_") ||
         startsWith(component.type, "limit_");
}

[[nodiscard]] bool isSwitchLike(const Component& component) {
  return startsWith(component.type, "switch_") || startsWith(component.type, "mushroom_") ||
         isSensorLike(component);
}

[[nodiscard]] bool isSelectorType(std::string_view type) {
  return type == component_type::kSelectorSwitch || startsWith(type, "selector_");
}

} // namespace

std::string simulationStateKey(const Component& component, const SimItemState& state) {
  using namespace component_type;
  if (isContactorContact(component.type)) {
    return state.closed ? "active" : std::string{};
  }
  if (isPushbutton(component.type) || isSwitchLike(component) || component.type == kEmergencyStop ||
      isSensorLike(component)) {
    return state.input_active ? "active" : std::string{};
  }
  if (component.type == kFuse || startsWith(component.type, "fuse")) {
    return state.tripped ? "blown" : std::string{};
  }
  if (component.type == kBreaker || startsWith(component.type, "breaker") ||
      startsWith(component.type, "rcd_") || component.type == "motor_breaker_3p" ||
      component.type == kThermalRelay) {
    return state.tripped ? "tripped" : std::string{};
  }
  if (isSelectorType(component.type)) {
    return "position_" + std::to_string(std::max(1, state.selector_position));
  }
  if (isTimer(component.type)) {
    return state.timer_output ? "active" : std::string{};
  }
  if (startsWith(component.type, "logic_") || startsWith(component.type, "display_")) {
    return state.logic_output ? "active" : std::string{};
  }
  if (isCoil(component.type) || component.type == kPilotLamp || component.type == kMotor3ph ||
      startsWith(component.type, "motor_") || startsWith(component.type, "lamp_") ||
      component.type == "buzzer" || component.type == "siren" || component.type == "bell" ||
      component.type == "horn" || component.type == "led" || component.type == "linear_actuator") {
    return state.energized ? "active" : std::string{};
  }
  return {};
}

QString componentStateText(const Component& component, const SimItemState& state) {
  using namespace component_type;
  if (isTimer(component.type)) {
    return state.timer_output ? QCoreApplication::translate("Simulation", "Timed out")
                              : QCoreApplication::translate("Simulation", "Timing");
  }
  if (startsWith(component.type, "logic_")) {
    return state.logic_output ? QCoreApplication::translate("Simulation", "Output ON")
                              : QCoreApplication::translate("Simulation", "Output OFF");
  }
  if (isCoil(component.type)) {
    return state.energized ? QCoreApplication::translate("Simulation", "Energized")
                           : QCoreApplication::translate("Simulation", "De-energized");
  }
  if (isContactorContact(component.type)) {
    return state.closed ? QCoreApplication::translate("Simulation", "Closed")
                        : QCoreApplication::translate("Simulation", "Open");
  }
  if (component.type == kPilotLamp || startsWith(component.type, "lamp_") ||
      component.type == "led") {
    return state.energized ? QCoreApplication::translate("Simulation", "Lit")
                           : QCoreApplication::translate("Simulation", "Off");
  }
  if (component.type == kMotor3ph || startsWith(component.type, "motor_")) {
    return state.energized ? QCoreApplication::translate("Simulation", "Running")
                           : QCoreApplication::translate("Simulation", "Stopped");
  }
  if (component.type == kEmergencyStop) {
    return state.input_active ? QCoreApplication::translate("Simulation", "Triggered (latched)")
                              : QCoreApplication::translate("Simulation", "Released");
  }
  if (isPushbutton(component.type) || isSwitchLike(component)) {
    return state.input_active ? QCoreApplication::translate("Simulation", "Pressed")
                              : QCoreApplication::translate("Simulation", "Released");
  }
  if (isSelectorType(component.type)) {
    return QCoreApplication::translate("Simulation", "Position %1").arg(state.selector_position);
  }
  if (component.type == kFuse || startsWith(component.type, "fuse")) {
    return state.tripped ? QCoreApplication::translate("Simulation", "Blown")
                         : QCoreApplication::translate("Simulation", "Intact");
  }
  if (component.type == kBreaker || startsWith(component.type, "breaker") ||
      startsWith(component.type, "rcd_") || component.type == kThermalRelay) {
    return state.tripped ? QCoreApplication::translate("Simulation", "Tripped")
                         : QCoreApplication::translate("Simulation", "Ready");
  }
  if (state.energized || state.closed || state.input_active) {
    return QCoreApplication::translate("Simulation", "Active");
  }
  return QCoreApplication::translate("Simulation", "No electrical state");
}

QString pinRoleText(PinRole role) {
  switch (role) {
  case PinRole::Terminal:
    return QCoreApplication::translate("PinRole", "Terminal");
  case PinRole::CoilInput:
    return QCoreApplication::translate("PinRole", "Coil input");
  case PinRole::CoilOutput:
    return QCoreApplication::translate("PinRole", "Coil output");
  case PinRole::ContactCommon:
    return QCoreApplication::translate("PinRole", "Common contact");
  case PinRole::ContactNo:
    return QCoreApplication::translate("PinRole", "Normally open contact");
  case PinRole::ContactNc:
    return QCoreApplication::translate("PinRole", "Normally closed contact");
  case PinRole::SupplyIn:
    return QCoreApplication::translate("PinRole", "Supply input");
  case PinRole::SupplyOut:
    return QCoreApplication::translate("PinRole", "Supply output");
  case PinRole::Neutral:
    return QCoreApplication::translate("PinRole", "Neutral");
  case PinRole::Earth:
    return QCoreApplication::translate("PinRole", "Earth");
  case PinRole::SignalIn:
    return QCoreApplication::translate("PinRole", "Signal input");
  case PinRole::SignalOut:
    return QCoreApplication::translate("PinRole", "Signal output");
  }
  return {};
}

QString categoryText(const std::string& category) {
  if (category == "supplies") {
    return QCoreApplication::translate("SymbolCategory", "Supplies");
  }
  if (category == "protection") {
    return QCoreApplication::translate("SymbolCategory", "Protection");
  }
  if (category == "contactors") {
    return QCoreApplication::translate("SymbolCategory", "Contactors");
  }
  if (category == "control") {
    return QCoreApplication::translate("SymbolCategory", "Control");
  }
  if (category == "timers") {
    return QCoreApplication::translate("SymbolCategory", "Timers");
  }
  if (category == "indication") {
    return QCoreApplication::translate("SymbolCategory", "Indication");
  }
  if (category == "machines") {
    return QCoreApplication::translate("SymbolCategory", "Machines");
  }
  if (category == "terminals") {
    return QCoreApplication::translate("SymbolCategory", "Terminals");
  }
  if (category == "annotations") {
    return QCoreApplication::translate("SymbolCategory", "Annotations");
  }
  if (category == "relays") {
    return QCoreApplication::translate("SymbolCategory", "Relays");
  }
  if (category == "contacts") {
    return QCoreApplication::translate("SymbolCategory", "Contacts");
  }
  if (category == "sensors") {
    return QCoreApplication::translate("SymbolCategory", "Sensors");
  }
  if (category == "power") {
    return QCoreApplication::translate("SymbolCategory", "Power");
  }
  if (category == "logic") {
    return QCoreApplication::translate("SymbolCategory", "Logic");
  }
  if (category == "plc") {
    return QCoreApplication::translate("SymbolCategory", "PLC and I/O");
  }
  if (category == "pneumatic") {
    return QCoreApplication::translate("SymbolCategory", "Pneumatics");
  }
  if (category == "measurement") {
    return QCoreApplication::translate("SymbolCategory", "Measurement");
  }
  if (category == "grafcet") {
    return QCoreApplication::translate("SymbolCategory", "GRAFCET");
  }
  return QString::fromStdString(category);
}

QString componentTypeText(const std::string& type, const SymbolLibrary* library, bool portuguese) {
  if (library != nullptr) {
    if (const SymbolDefinition* definition = library->find(type)) {
      return QString::fromStdString(definition->displayName(portuguese));
    }
  }
  return QString::fromStdString(type);
}

QString formatSimulatedTime(qint64 milliseconds) {
  if (milliseconds < 1000) {
    return QCoreApplication::translate("Simulation", "%1 ms").arg(milliseconds);
  }
  return QCoreApplication::translate("Simulation", "%1 s")
      .arg(QString::number(static_cast<double>(milliseconds) / 1000.0, 'f', 2));
}

QString formatNumber(double value, int decimals) {
  return QLocale().toString(value, 'f', decimals);
}

} // namespace fiovivo::ui
