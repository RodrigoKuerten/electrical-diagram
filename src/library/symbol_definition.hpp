#pragma once

#include "core/ids.hpp"
#include "core/model.hpp"
#include "core/result.hpp"
#include "core/types.hpp"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fiovivo {

enum class ParameterKind { Boolean, Integer, Number, String, Enum };

struct ParameterDefinition {
  std::string key;
  ParameterKind kind = ParameterKind::String;
  std::string label;
  std::string label_pt;
  std::string unit;
  std::vector<std::string> options;
  std::optional<double> minimum;
  std::optional<double> maximum;

  [[nodiscard]] std::string displayLabel(bool portuguese) const {
    return portuguese && !label_pt.empty() ? label_pt : label;
  }
};

struct PinDefinition {
  std::string name;
  PinRole role = PinRole::Terminal;
  Vec2 offset;
  bool required = false;

  friend bool operator==(const PinDefinition&, const PinDefinition&) = default;
};

struct SymbolDefinition {
  int symbol_version = 1;
  std::string type;
  std::string display_name;
  std::string display_name_pt;
  std::string category;
  Vec2 size{40.0, 40.0};
  std::vector<PinDefinition> pins;
  ParameterMap default_parameters;
  std::vector<ParameterDefinition> parameter_definitions;
  std::map<std::string, std::string> states;
  std::map<std::string, std::vector<std::string>> element_ids;

  [[nodiscard]] const PinDefinition* findPin(std::string_view name) const;
  [[nodiscard]] std::string displayName(bool portuguese) const {
    return portuguese && !display_name_pt.empty() ? display_name_pt : display_name;
  }
  [[nodiscard]] const ParameterDefinition* findParameter(std::string_view key) const;

  friend bool operator==(const SymbolDefinition&, const SymbolDefinition&) = default;
};

class SymbolLibrary {
public:
  Result<void> loadFromDirectory(const std::filesystem::path& directory);

  [[nodiscard]] const SymbolDefinition* find(std::string_view type) const;
  [[nodiscard]] const std::map<std::string, SymbolDefinition>& definitions() const {
    return definitions_;
  }
  [[nodiscard]] const std::filesystem::path& directory() const { return directory_; }
  [[nodiscard]] bool empty() const { return definitions_.empty(); }
  [[nodiscard]] std::vector<std::string> categories() const;

  [[nodiscard]] static std::string defaultTagBase(std::string_view type);

private:
  std::map<std::string, SymbolDefinition> definitions_;
  std::filesystem::path directory_;
};

// Builds a model component from a symbol definition. IDs are allocated from the diagram.
[[nodiscard]] Component instantiateSymbol(const SymbolDefinition& definition,
                                          Diagram& diagram,
                                          Vec2 position,
                                          std::string tag);

} // namespace fiovivo
