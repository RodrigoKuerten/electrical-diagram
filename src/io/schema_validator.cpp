#include "io/schema_validator.hpp"

#include "io/schema_embedded.hpp"

#include <cmath>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace fiovivo::io {
namespace {

[[nodiscard]] std::string joinPath(const std::string& path, std::string_view component) {
  if (path.empty()) {
    return "/" + std::string(component);
  }
  return path + "/" + std::string(component);
}

[[nodiscard]] std::string joinIndex(const std::string& path, std::size_t index) {
  if (path.empty()) {
    return "/" + std::to_string(index);
  }
  return path + "/" + std::to_string(index);
}

[[nodiscard]] bool matchesType(const nlohmann::json& value, std::string_view type) {
  if (type == "object") {
    return value.is_object();
  }
  if (type == "array") {
    return value.is_array();
  }
  if (type == "string") {
    return value.is_string();
  }
  if (type == "boolean") {
    return value.is_boolean();
  }
  if (type == "integer") {
    return value.is_number_integer();
  }
  if (type == "number") {
    return value.is_number();
  }
  if (type == "null") {
    return value.is_null();
  }
  return true;
}

[[nodiscard]] Result<void> validate(const nlohmann::json& value,
                                    const nlohmann::json& schema,
                                    const nlohmann::json& root,
                                    const std::string& path);

[[nodiscard]] Result<void> validateRef(const nlohmann::json& value,
                                       const nlohmann::json& schema,
                                       const nlohmann::json& root,
                                       const std::string& path) {
  const std::string reference = schema.at("$ref").get<std::string>();
  if (reference.empty() || reference[0] != '#') {
    return makeError(
        ErrorCode::SchemaViolation, "Unsupported non-local JSON schema reference", reference);
  }
  try {
    const nlohmann::json& target = root.at(nlohmann::json::json_pointer(reference.substr(1)));
    return validate(value, target, root, path);
  } catch (const nlohmann::json::exception& error) {
    return makeError(
        ErrorCode::SchemaViolation, "Unresolvable JSON schema reference", error.what());
  }
}

[[nodiscard]] Result<void>
validateType(const nlohmann::json& value, const nlohmann::json& schema, const std::string& path) {
  if (!schema.contains("type")) {
    return {};
  }
  const nlohmann::json& type = schema.at("type");
  bool matched = false;
  if (type.is_string()) {
    matched = matchesType(value, type.get<std::string>());
  } else if (type.is_array()) {
    for (const nlohmann::json& item : type) {
      if (item.is_string() && matchesType(value, item.get<std::string>())) {
        matched = true;
        break;
      }
    }
  } else {
    return makeError(ErrorCode::SchemaViolation, "Schema type declaration is invalid", path);
  }
  if (!matched) {
    return makeError(ErrorCode::SchemaViolation,
                     "Value has the wrong JSON type at " + (path.empty() ? "/" : path));
  }
  return {};
}

[[nodiscard]] Result<void>
validateEnum(const nlohmann::json& value, const nlohmann::json& schema, const std::string& path) {
  if (!schema.contains("enum")) {
    return {};
  }
  const nlohmann::json& options = schema.at("enum");
  for (const nlohmann::json& option : options) {
    if (value == option) {
      return {};
    }
  }
  return makeError(ErrorCode::SchemaViolation,
                   "Value is not one of the allowed options at " + (path.empty() ? "/" : path));
}

[[nodiscard]] Result<void> validateObject(const nlohmann::json& value,
                                          const nlohmann::json& schema,
                                          const nlohmann::json& root,
                                          const std::string& path) {
  if (!value.is_object()) {
    return {};
  }
  if (schema.contains("required")) {
    for (const nlohmann::json& required : schema.at("required")) {
      const std::string name = required.get<std::string>();
      if (!value.contains(name)) {
        return makeError(ErrorCode::SchemaViolation,
                         "Missing required field at " + joinPath(path, name));
      }
    }
  }
  const nlohmann::json* properties = nullptr;
  if (schema.contains("properties")) {
    properties = &schema.at("properties");
    for (auto iterator = properties->begin(); iterator != properties->end(); ++iterator) {
      if (!value.contains(iterator.key())) {
        continue;
      }
      const auto result = validate(
          value.at(iterator.key()), iterator.value(), root, joinPath(path, iterator.key()));
      if (!result) {
        return result;
      }
    }
  }
  if (schema.contains("additionalProperties") && schema.at("additionalProperties").is_boolean() &&
      !schema.at("additionalProperties").get<bool>()) {
    for (auto iterator = value.begin(); iterator != value.end(); ++iterator) {
      if (properties == nullptr || !properties->contains(iterator.key())) {
        return makeError(ErrorCode::SchemaViolation,
                         "Unexpected field at " + joinPath(path, iterator.key()));
      }
    }
  }
  return {};
}

[[nodiscard]] Result<void> validateArray(const nlohmann::json& value,
                                         const nlohmann::json& schema,
                                         const nlohmann::json& root,
                                         const std::string& path) {
  if (!value.is_array()) {
    return {};
  }
  if (schema.contains("minItems") && value.size() < schema.at("minItems").get<std::size_t>()) {
    return makeError(ErrorCode::SchemaViolation,
                     "Array is too short at " + (path.empty() ? "/" : path));
  }
  if (schema.contains("maxItems") && value.size() > schema.at("maxItems").get<std::size_t>()) {
    return makeError(ErrorCode::SchemaViolation,
                     "Array is too long at " + (path.empty() ? "/" : path));
  }
  if (schema.contains("items")) {
    for (std::size_t index = 0; index < value.size(); ++index) {
      const auto result = validate(value[index], schema.at("items"), root, joinIndex(path, index));
      if (!result) {
        return result;
      }
    }
  }
  return {};
}

[[nodiscard]] Result<void> validateScalarConstraints(const nlohmann::json& value,
                                                     const nlohmann::json& schema,
                                                     const std::string& path) {
  if (schema.contains("const") && value != schema.at("const")) {
    return makeError(ErrorCode::SchemaViolation,
                     "Value does not match the required constant at " +
                         (path.empty() ? "/" : path));
  }
  if (value.is_string()) {
    const std::string text = value.get<std::string>();
    if (schema.contains("minLength") && text.size() < schema.at("minLength").get<std::size_t>()) {
      return makeError(ErrorCode::SchemaViolation,
                       "String is too short at " + (path.empty() ? "/" : path));
    }
    if (schema.contains("maxLength") && text.size() > schema.at("maxLength").get<std::size_t>()) {
      return makeError(ErrorCode::SchemaViolation,
                       "String is too long at " + (path.empty() ? "/" : path));
    }
    if (schema.contains("pattern")) {
      const std::string pattern = schema.at("pattern").get<std::string>();
      if (!std::regex_match(text, std::regex(pattern))) {
        return makeError(ErrorCode::SchemaViolation,
                         "String does not match pattern at " + (path.empty() ? "/" : path));
      }
    }
  }
  if (value.is_number()) {
    if (schema.contains("minimum")) {
      const double minimum = schema.at("minimum").get<double>();
      if (value.get<double>() < minimum) {
        return makeError(ErrorCode::SchemaViolation,
                         "Number is below the minimum at " + (path.empty() ? "/" : path));
      }
    }
    if (schema.contains("maximum")) {
      const double maximum = schema.at("maximum").get<double>();
      if (value.get<double>() > maximum) {
        return makeError(ErrorCode::SchemaViolation,
                         "Number is above the maximum at " + (path.empty() ? "/" : path));
      }
    }
  }
  return {};
}

Result<void> validate(const nlohmann::json& value,
                      const nlohmann::json& schema,
                      const nlohmann::json& root,
                      const std::string& path) {
  if (!schema.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Invalid JSON schema node", path);
  }
  if (schema.contains("$ref")) {
    return validateRef(value, schema, root, path);
  }
  if (const auto result = validateType(value, schema, path); !result) {
    return result;
  }
  if (const auto result = validateEnum(value, schema, path); !result) {
    return result;
  }
  if (const auto result = validateScalarConstraints(value, schema, path); !result) {
    return result;
  }
  if (const auto result = validateObject(value, schema, root, path); !result) {
    return result;
  }
  if (const auto result = validateArray(value, schema, root, path); !result) {
    return result;
  }
  return {};
}

} // namespace

Result<void> validateAgainstSchema(const nlohmann::json& document, const nlohmann::json& schema) {
  return validate(document, schema, schema, "");
}

const nlohmann::json& bundledSchema() {
  static const nlohmann::json schema = [] {
    try {
      return nlohmann::json::parse(detail::kFileFormatSchemaJson);
    } catch (const nlohmann::json::exception&) {
      return nlohmann::json::object();
    }
  }();
  return schema;
}

} // namespace fiovivo::io
