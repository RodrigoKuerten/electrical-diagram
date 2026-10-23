#pragma once

#include "core/geometry.hpp"
#include "core/ids.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace fiovivo {

enum class PinRole {
  Terminal,
  CoilInput,
  CoilOutput,
  ContactCommon,
  ContactNo,
  ContactNc,
  SupplyIn,
  SupplyOut,
  Neutral,
  Earth,
  SignalIn,
  SignalOut,
};

[[nodiscard]] std::string_view toString(PinRole role);
[[nodiscard]] std::optional<PinRole> pinRoleFromString(std::string_view value);
[[nodiscard]] bool isSourceRole(PinRole role);

enum class PaperSize { A4, A3, A2, A1, Letter };
enum class Orientation { Landscape, Portrait };

[[nodiscard]] std::string_view toString(PaperSize size);
[[nodiscard]] std::string_view toString(Orientation orientation);
[[nodiscard]] std::optional<PaperSize> paperSizeFromString(std::string_view value);
[[nodiscard]] std::optional<Orientation> orientationFromString(std::string_view value);
[[nodiscard]] Vec2 paperDimensions(PaperSize size, Orientation orientation);

enum class WireDash { Solid, Dashed, Dotted };
[[nodiscard]] std::string_view toString(WireDash dash);
[[nodiscard]] std::optional<WireDash> wireDashFromString(std::string_view value);

enum class AnnotationKind { Text, Rectangle, Legend };
[[nodiscard]] std::string_view toString(AnnotationKind kind);
[[nodiscard]] std::optional<AnnotationKind> annotationKindFromString(std::string_view value);

using ParameterValue = std::variant<bool, std::int64_t, double, std::string>;
using ParameterMap = std::map<std::string, ParameterValue>;
using UnknownFields = std::map<std::string, std::string>;

[[nodiscard]] bool
parameterBool(const ParameterMap& parameters, std::string_view key, bool fallback = false);
[[nodiscard]] std::int64_t
parameterInt(const ParameterMap& parameters, std::string_view key, std::int64_t fallback = 0);
[[nodiscard]] double
parameterNumber(const ParameterMap& parameters, std::string_view key, double fallback = 0.0);
[[nodiscard]] std::string
parameterString(const ParameterMap& parameters, std::string_view key, std::string fallback = {});
[[nodiscard]] const ParameterValue* findParameter(const ParameterMap& parameters,
                                                  std::string_view key);

struct Metadata {
  std::string title;
  std::string author;
  std::string created_utc;
  std::string modified_utc;
  std::string revision = "A";

  friend bool operator==(const Metadata&, const Metadata&) = default;
};

struct Labels {
  bool show_tag = true;
  bool show_description = false;
  Vec2 tag_offset{0.0, -6.0};
  Vec2 description_offset{0.0, 6.0};

  friend bool operator==(const Labels&, const Labels&) = default;
};

struct WireStyle {
  std::string color = "#1f2933";
  double thickness = 0.8;
  WireDash dash = WireDash::Solid;

  friend bool operator==(const WireStyle&, const WireStyle&) = default;
};

namespace component_type {
inline constexpr std::string_view kSupplyAc3 = "supply_ac3";
inline constexpr std::string_view kSupplyAc1 = "supply_ac1";
inline constexpr std::string_view kSupplyDc = "supply_dc";
inline constexpr std::string_view kFuse = "fuse";
inline constexpr std::string_view kBreaker = "breaker";
inline constexpr std::string_view kContactorCoil = "contactor_coil";
inline constexpr std::string_view kContactorNo = "contactor_no";
inline constexpr std::string_view kContactorAuxNo = "contactor_aux_no";
inline constexpr std::string_view kContactorAuxNc = "contactor_aux_nc";
inline constexpr std::string_view kPushbuttonNo = "pushbutton_no";
inline constexpr std::string_view kPushbuttonNc = "pushbutton_nc";
inline constexpr std::string_view kEmergencyStop = "emergency_stop";
inline constexpr std::string_view kSelectorSwitch = "selector_switch";
inline constexpr std::string_view kThermalRelay = "thermal_relay";
inline constexpr std::string_view kTimerOn = "timer_on";
inline constexpr std::string_view kTimerOff = "timer_off";
inline constexpr std::string_view kPilotLamp = "pilot_lamp";
inline constexpr std::string_view kMotor3ph = "motor_3ph";
inline constexpr std::string_view kTerminal = "terminal";
inline constexpr std::string_view kLabelBox = "label_box";
} // namespace component_type

enum class ComponentCategory {
  Supply,
  Protection,
  Contactor,
  Control,
  Timer,
  Indication,
  Machine,
  Terminal,
  Annotation,
  Unknown,
};

[[nodiscard]] ComponentCategory categorizeComponent(std::string_view type);
[[nodiscard]] bool isContactorContact(std::string_view type);
[[nodiscard]] bool isPushbutton(std::string_view type);
[[nodiscard]] bool isTimer(std::string_view type);
[[nodiscard]] bool isCoil(std::string_view type);
[[nodiscard]] bool isSupply(std::string_view type);
[[nodiscard]] bool isLoad(std::string_view type);
[[nodiscard]] bool isUserActuated(std::string_view type);

struct PaperSetup {
  PaperSize paper = PaperSize::A4;
  Orientation orientation = Orientation::Landscape;
  double margin = 10.0;
  bool show_frame = true;

  friend bool operator==(const PaperSetup&, const PaperSetup&) = default;
};

} // namespace fiovivo
