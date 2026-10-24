#pragma once

#include "core/model.hpp"
#include "core/types.hpp"
#include "ui/sim_ui_types.hpp"

#include <QString>

#include <string>

namespace fiovivo {
class SymbolLibrary;
} // namespace fiovivo

namespace fiovivo::ui {

[[nodiscard]] std::string simulationStateKey(const Component& component, const SimItemState& state);
[[nodiscard]] QString componentStateText(const Component& component, const SimItemState& state);
[[nodiscard]] QString pinRoleText(PinRole role);
[[nodiscard]] QString categoryText(const std::string& category);
[[nodiscard]] QString
componentTypeText(const std::string& type, const fiovivo::SymbolLibrary* library, bool portuguese);
[[nodiscard]] QString formatSimulatedTime(qint64 milliseconds);
[[nodiscard]] QString formatNumber(double value, int decimals = 2);

} // namespace fiovivo::ui
