#pragma once

#include "core/model.hpp"
#include "core/result.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace fiovivo::io {

inline constexpr std::string_view kFormatName = "dcj";
inline constexpr int kFormatVersion = 1;
inline constexpr std::string_view kAppVersion = "0.1.0";

[[nodiscard]] Result<nlohmann::json> toJson(const Diagram& diagram);
[[nodiscard]] Result<nlohmann::json> toJson(const Diagram& diagram, int indent);
[[nodiscard]] Result<std::string> toJsonString(const Diagram& diagram);
[[nodiscard]] Result<std::string> toJsonString(const Diagram& diagram, int indent);

[[nodiscard]] Result<Diagram> fromJson(const nlohmann::json& document);
[[nodiscard]] Result<Diagram> fromJsonString(std::string_view text);

[[nodiscard]] Result<Diagram> loadFile(const std::filesystem::path& path);
[[nodiscard]] Result<void> saveFile(const Diagram& diagram, const std::filesystem::path& path);

// Deterministic ID formatting used by the .dcj format. IDs are strings such as c12, p4, w7, s1.
[[nodiscard]] std::string idToString(ComponentId id);
[[nodiscard]] std::string idToString(PinId id);
[[nodiscard]] std::string idToString(WireId id);
[[nodiscard]] std::string idToString(SheetId id);
[[nodiscard]] std::string idToString(AnnotationId id);

} // namespace fiovivo::io
