#include "ui/ui_helpers.hpp"

#include "library/symbol_definition.hpp"

#include <QCoreApplication>
#include <QLocale>

namespace fiovivo::ui {

std::string simulationStateKey(const Component& component, const SimItemState& state) {
  using namespace component_type;
  if (component.type == kContactorNo || component.type == kContactorAuxNo ||
      component.type == kContactorAuxNc) {
    return state.closed ? "active" : std::string{};
  }
  if (component.type == kPushbuttonNo || component.type == kPushbuttonNc ||
      component.type == kEmergencyStop) {
    return state.input_active ? "active" : std::string{};
  }
  if (component.type == kFuse) {
    return state.tripped ? "blown" : std::string{};
  }
  if (component.type == kBreaker || component.type == kThermalRelay) {
    return state.tripped ? "tripped" : std::string{};
  }
  if (component.type == kSelectorSwitch) {
    return "position_" + std::to_string(std::max(1, state.selector_position));
  }
  if (component.type == kTimerOn || component.type == kTimerOff) {
    return state.timer_output ? "active" : std::string{};
  }
  if (component.type == kContactorCoil || component.type == kPilotLamp ||
      component.type == kMotor3ph) {
    return state.energized ? "active" : std::string{};
  }
  return {};
}

QString componentStateText(const Component& component, const SimItemState& state) {
  using namespace component_type;
  if (component.type == kTimerOn || component.type == kTimerOff) {
    return state.timer_output ? QCoreApplication::translate("Simulation", "Timed out")
                              : QCoreApplication::translate("Simulation", "Timing");
  }
  if (component.type == kContactorCoil) {
    return state.energized ? QCoreApplication::translate("Simulation", "Energized")
                           : QCoreApplication::translate("Simulation", "De-energized");
  }
  if (component.type == kContactorNo || component.type == kContactorAuxNo ||
      component.type == kContactorAuxNc) {
    return state.closed ? QCoreApplication::translate("Simulation", "Closed")
                        : QCoreApplication::translate("Simulation", "Open");
  }
  if (component.type == kPilotLamp) {
    return state.energized ? QCoreApplication::translate("Simulation", "Lit")
                           : QCoreApplication::translate("Simulation", "Off");
  }
  if (component.type == kMotor3ph) {
    return state.energized ? QCoreApplication::translate("Simulation", "Running")
                           : QCoreApplication::translate("Simulation", "Stopped");
  }
  if (component.type == kEmergencyStop) {
    return state.input_active ? QCoreApplication::translate("Simulation", "Triggered (latched)")
                              : QCoreApplication::translate("Simulation", "Released");
  }
  if (component.type == kPushbuttonNo || component.type == kPushbuttonNc) {
    return state.input_active ? QCoreApplication::translate("Simulation", "Pressed")
                              : QCoreApplication::translate("Simulation", "Released");
  }
  if (component.type == kSelectorSwitch) {
    return QCoreApplication::translate("Simulation", "Position %1").arg(state.selector_position);
  }
  if (component.type == kFuse) {
    return state.tripped ? QCoreApplication::translate("Simulation", "Blown")
                         : QCoreApplication::translate("Simulation", "Intact");
  }
  if (component.type == kBreaker || component.type == kThermalRelay) {
    return state.tripped ? QCoreApplication::translate("Simulation", "Tripped")
                         : QCoreApplication::translate("Simulation", "Ready");
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
