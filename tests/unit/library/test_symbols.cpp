#include "core/model.hpp"
#include "core/types.hpp"
#include "library/symbol_definition.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <set>
#include <string>

#ifndef FIOVIVO_ASSET_DIR
  #define FIOVIVO_ASSET_DIR "assets"
#endif

namespace fiovivo {
namespace {

[[nodiscard]] std::filesystem::path symbolDirectory() {
  return std::filesystem::path(FIOVIVO_ASSET_DIR) / "symbols";
}

TEST(SymbolLibraryTest, LoadsAllRequiredDefinitions) {
  SymbolLibrary library;
  const auto result = library.loadFromDirectory(symbolDirectory());
  ASSERT_TRUE(result) << result.error().toString();
  EXPECT_EQ(library.definitions().size(), 20U);
  for (const std::string_view type :
       {component_type::kSupplyAc3,      component_type::kSupplyAc1,
        component_type::kSupplyDc,       component_type::kFuse,
        component_type::kBreaker,        component_type::kContactorCoil,
        component_type::kContactorNo,    component_type::kContactorAuxNo,
        component_type::kContactorAuxNc, component_type::kPushbuttonNo,
        component_type::kPushbuttonNc,   component_type::kEmergencyStop,
        component_type::kSelectorSwitch, component_type::kThermalRelay,
        component_type::kTimerOn,        component_type::kTimerOff,
        component_type::kPilotLamp,      component_type::kMotor3ph,
        component_type::kTerminal,       component_type::kLabelBox}) {
    EXPECT_NE(library.find(type), nullptr) << type;
  }
}

TEST(SymbolLibraryTest, EveryDefinitionHasGeometryAndVersionOne) {
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory(symbolDirectory()));
  for (const auto& [type, definition] : library.definitions()) {
    EXPECT_EQ(definition.symbol_version, 1) << type;
    EXPECT_FALSE(definition.display_name.empty()) << type;
    EXPECT_FALSE(definition.display_name_pt.empty()) << type;
    EXPECT_GT(definition.size.x, 0.0) << type;
    EXPECT_GT(definition.size.y, 0.0) << type;
    EXPECT_TRUE(std::filesystem::is_regular_file(symbolDirectory() / (type + ".svg"))) << type;
    for (const auto& [state, filename] : definition.states) {
      (void)state;
      EXPECT_TRUE(std::filesystem::is_regular_file(symbolDirectory() / filename))
          << type << " -> " << filename;
    }
    EXPECT_GE(definition.pins.size(), 0U);
    for (const PinDefinition& pin : definition.pins) {
      EXPECT_FALSE(pin.name.empty()) << type;
      EXPECT_GE(pin.offset.x, 0.0) << type << "/" << pin.name;
      EXPECT_GE(pin.offset.y, 0.0) << type << "/" << pin.name;
      EXPECT_LE(pin.offset.x, definition.size.x) << type << "/" << pin.name;
      EXPECT_LE(pin.offset.y, definition.size.y) << type << "/" << pin.name;
    }
  }
}

TEST(SymbolLibraryTest, RequiredPinsMatchTheSpecification) {
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory(symbolDirectory()));
  const auto check = [&library](std::string_view type,
                                std::vector<std::pair<std::string, PinRole>> pins) {
    const SymbolDefinition* definition = library.find(type);
    ASSERT_NE(definition, nullptr);
    EXPECT_EQ(definition->pins.size(), pins.size()) << type;
    for (const auto& [name, role] : pins) {
      const PinDefinition* pin = definition->findPin(name);
      ASSERT_NE(pin, nullptr) << type << "/" << name;
      EXPECT_EQ(pin->role, role) << type << "/" << name;
    }
  };
  check(component_type::kSupplyAc3,
        {{"L1", PinRole::SupplyIn},
         {"L2", PinRole::SupplyIn},
         {"L3", PinRole::SupplyIn},
         {"N", PinRole::Neutral},
         {"PE", PinRole::Earth}});
  check(component_type::kContactorCoil, {{"A1", PinRole::CoilInput}, {"A2", PinRole::CoilOutput}});
  check(component_type::kContactorAuxNo,
        {{"13", PinRole::ContactCommon}, {"14", PinRole::ContactNo}});
  check(component_type::kContactorAuxNc,
        {{"21", PinRole::ContactCommon}, {"22", PinRole::ContactNc}});
  check(component_type::kThermalRelay,
        {{"95", PinRole::ContactCommon},
         {"96", PinRole::ContactNc},
         {"97", PinRole::ContactCommon},
         {"98", PinRole::ContactNo}});
  check(component_type::kTimerOn,
        {{"A1", PinRole::CoilInput},
         {"A2", PinRole::CoilOutput},
         {"15", PinRole::ContactCommon},
         {"18", PinRole::ContactNo}});
  check(component_type::kMotor3ph,
        {{"U1", PinRole::SupplyIn}, {"V1", PinRole::SupplyIn}, {"W1", PinRole::SupplyIn}});
}

TEST(SymbolLibraryTest, StateVariantsCoverDynamicSymbols) {
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory(symbolDirectory()));
  const auto has_state = [&library](std::string_view type, std::string_view state) {
    const SymbolDefinition* definition = library.find(type);
    ASSERT_NE(definition, nullptr);
    EXPECT_TRUE(definition->states.contains(std::string(state))) << type << "/" << state;
  };
  has_state(component_type::kContactorCoil, "active");
  has_state(component_type::kContactorNo, "active");
  has_state(component_type::kContactorAuxNc, "active");
  has_state(component_type::kPushbuttonNo, "active");
  has_state(component_type::kEmergencyStop, "active");
  has_state(component_type::kFuse, "blown");
  has_state(component_type::kBreaker, "tripped");
  has_state(component_type::kThermalRelay, "tripped");
  has_state(component_type::kSelectorSwitch, "position_2");
  has_state(component_type::kTimerOn, "active");
  has_state(component_type::kPilotLamp, "active");
  has_state(component_type::kMotor3ph, "active");
}

TEST(SymbolLibraryTest, InstantiateCreatesComponentAndFreshIds) {
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory(symbolDirectory()));
  const SymbolDefinition* definition = library.find(component_type::kContactorCoil);
  ASSERT_NE(definition, nullptr);
  Diagram diagram;
  Component first = instantiateSymbol(*definition, diagram, {10.0, 20.0}, "KM1");
  Component second = instantiateSymbol(*definition, diagram, {50.0, 20.0}, "KM2");
  EXPECT_NE(first.id, second.id);
  EXPECT_EQ(first.pins.size(), 2U);
  EXPECT_NE(first.pins[0].id, second.pins[0].id);
  EXPECT_EQ(first.size, definition->size);
  EXPECT_EQ(first.parameters, definition->default_parameters);
  EXPECT_EQ(first.tag, "KM1");
  EXPECT_EQ(first.position, (Vec2{10.0, 20.0}));
  EXPECT_EQ(first.pins[0].role, PinRole::CoilInput);
}

TEST(SymbolLibraryTest, DefaultTagsMapToDomainPrefixes) {
  EXPECT_EQ(SymbolLibrary::defaultTagBase(component_type::kContactorCoil), "KM1");
  EXPECT_EQ(SymbolLibrary::defaultTagBase(component_type::kContactorAuxNo), "KM1.1");
  EXPECT_EQ(SymbolLibrary::defaultTagBase(component_type::kPilotLamp), "H1");
  EXPECT_EQ(SymbolLibrary::defaultTagBase(component_type::kMotor3ph), "M1");
  EXPECT_EQ(SymbolLibrary::defaultTagBase(component_type::kTimerOn), "TM1");
}

TEST(SymbolLibraryTest, RejectsMissingDirectory) {
  SymbolLibrary library;
  const auto result = library.loadFromDirectory(symbolDirectory() / "does-not-exist");
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error().code, ErrorCode::IoError);
}

} // namespace
} // namespace fiovivo
