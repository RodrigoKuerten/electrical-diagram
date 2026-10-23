#include "io/json_io.hpp"

#include "core/types.hpp"
#include "io/schema_validator.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fiovivo::io {
namespace {

using nlohmann::json;

constexpr char kComponentPrefix = 'c';
constexpr char kPinPrefix = 'p';
constexpr char kWirePrefix = 'w';
constexpr char kSheetPrefix = 's';
constexpr char kAnnotationPrefix = 'a';

template<typename Id>
[[nodiscard]] std::string formatId(char prefix, Id id) {
  return std::string(1, prefix) + std::to_string(id.value());
}

template<typename Id>
[[nodiscard]] Result<Id> parseId(const json& value, char prefix, const std::string& path) {
  if (!value.is_string()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a string id", path);
  }
  const std::string text = value.get<std::string>();
  if (text.size() < 2 || text.front() != prefix) {
    return makeError(
        ErrorCode::SchemaViolation, "Expected an id such as " + std::string(1, prefix) + "1", path);
  }
  std::uint64_t number = 0;
  const char* begin = text.data() + 1;
  const char* end = text.data() + text.size();
  const auto result = std::from_chars(begin, end, number);
  if (result.ec != std::errc{} || result.ptr != end || number == 0) {
    return makeError(ErrorCode::SchemaViolation, "Invalid id: " + text, path);
  }
  return Id{number};
}

template<std::size_t N>
[[nodiscard]] bool isKnownField(std::string_view key,
                                const std::array<std::string_view, N>& known) {
  for (std::string_view item : known) {
    if (item == key) {
      return true;
    }
  }
  return false;
}

template<std::size_t N>
[[nodiscard]] UnknownFields captureUnknown(const json& object,
                                           const std::array<std::string_view, N>& known) {
  UnknownFields unknown;
  if (!object.is_object()) {
    return unknown;
  }
  for (auto iterator = object.begin(); iterator != object.end(); ++iterator) {
    if (!isKnownField(iterator.key(), known)) {
      unknown[iterator.key()] = iterator.value().dump();
    }
  }
  return unknown;
}

[[nodiscard]] Result<void>
mergeUnknown(json& target, const UnknownFields& unknown, const std::string& path) {
  for (const auto& [key, raw] : unknown) {
    try {
      target[key] = json::parse(raw);
    } catch (const json::exception& error) {
      return makeError(ErrorCode::ParseError,
                       "Invalid preserved JSON field: " + key,
                       path + " (" + error.what() + ")");
    }
  }
  return {};
}

[[nodiscard]] Result<Vec2> parsePoint(const json& value, const std::string& path) {
  if (!value.is_array() || value.size() < 2 || !value[0].is_number() || !value[1].is_number()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a point [x, y]", path);
  }
  return Vec2{value[0].get<double>(), value[1].get<double>()};
}

[[nodiscard]] json pointJson(Vec2 point) { return json::array({point.x, point.y}); }

[[nodiscard]] json sizeJson(Vec2 size) { return json::array({size.x, size.y}); }

[[nodiscard]] Result<ParameterMap> parseParameters(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected an object", path);
  }
  ParameterMap parameters;
  for (auto iterator = value.begin(); iterator != value.end(); ++iterator) {
    const json& item = iterator.value();
    const std::string item_path = path + "/" + iterator.key();
    if (item.is_boolean()) {
      parameters[iterator.key()] = item.get<bool>();
    } else if (item.is_number_unsigned()) {
      const std::uint64_t raw = item.get<std::uint64_t>();
      if (raw > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return makeError(ErrorCode::SchemaViolation, "Integer parameter is too large", item_path);
      }
      parameters[iterator.key()] = static_cast<std::int64_t>(raw);
    } else if (item.is_number_integer()) {
      parameters[iterator.key()] = item.get<std::int64_t>();
    } else if (item.is_number_float()) {
      parameters[iterator.key()] = item.get<double>();
    } else if (item.is_string()) {
      parameters[iterator.key()] = item.get<std::string>();
    } else {
      return makeError(ErrorCode::SchemaViolation,
                       "Parameter values must be bool, integer, number or string",
                       item_path);
    }
  }
  return parameters;
}

[[nodiscard]] json parametersJson(const ParameterMap& parameters) {
  json result = json::object();
  for (const auto& [key, value] : parameters) {
    if (const auto* boolean = std::get_if<bool>(&value)) {
      result[key] = *boolean;
    } else if (const auto* integer = std::get_if<std::int64_t>(&value)) {
      result[key] = *integer;
    } else if (const auto* number = std::get_if<double>(&value)) {
      result[key] = *number;
    } else if (const auto* text = std::get_if<std::string>(&value)) {
      result[key] = *text;
    }
  }
  return result;
}

constexpr std::array<std::string_view, 6> kRootFields{
    "format", "format_version", "app_version", "metadata", "sheets", "library_refs"};
constexpr std::array<std::string_view, 5> kMetadataFields{
    "title", "author", "created_utc", "modified_utc", "revision"};
constexpr std::array<std::string_view, 9> kSheetFields{"id",
                                                       "name",
                                                       "paper",
                                                       "orientation",
                                                       "margin",
                                                       "show_frame",
                                                       "components",
                                                       "wires",
                                                       "annotations"};
constexpr std::array<std::string_view, 10> kComponentFields{"id",
                                                            "type",
                                                            "tag",
                                                            "description",
                                                            "position",
                                                            "size",
                                                            "rotation",
                                                            "pins",
                                                            "parameters",
                                                            "labels"};
constexpr std::array<std::string_view, 5> kPinFields{"id", "name", "role", "offset", "required"};
constexpr std::array<std::string_view, 6> kWireFields{
    "id", "from", "to", "route", "style", "allow_self_loop"};
constexpr std::array<std::string_view, 2> kEndpointFields{"component", "pin"};
constexpr std::array<std::string_view, 3> kStyleFields{"color", "thickness", "dash"};
constexpr std::array<std::string_view, 4> kLabelFields{
    "show_tag", "show_description", "tag_offset", "description_offset"};
constexpr std::array<std::string_view, 7> kAnnotationFields{
    "id", "kind", "position", "size", "text", "font_size", "color"};

[[nodiscard]] Result<Labels> parseLabels(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a labels object", path);
  }
  Labels labels;
  labels.show_tag = value.value("show_tag", true);
  labels.show_description = value.value("show_description", false);
  const auto tag_offset = parsePoint(value.at("tag_offset"), path + "/tag_offset");
  if (!tag_offset) {
    return tag_offset.error();
  }
  labels.tag_offset = tag_offset.value();
  const auto description_offset =
      parsePoint(value.at("description_offset"), path + "/description_offset");
  if (!description_offset) {
    return description_offset.error();
  }
  labels.description_offset = description_offset.value();
  labels.unknown_fields = captureUnknown(value, kLabelFields);
  return labels;
}

[[nodiscard]] Result<json> labelsJson(const Labels& labels) {
  json result = json::object();
  const auto merged = mergeUnknown(result, labels.unknown_fields, "labels");
  if (!merged) {
    return merged.error();
  }
  result["show_tag"] = labels.show_tag;
  result["show_description"] = labels.show_description;
  result["tag_offset"] = pointJson(labels.tag_offset);
  result["description_offset"] = pointJson(labels.description_offset);
  return result;
}

[[nodiscard]] Result<WireStyle> parseStyle(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a wire style object", path);
  }
  WireStyle style;
  style.color = value.value("color", std::string("#1f2933"));
  style.thickness = value.value("thickness", 0.8);
  const std::string dash = value.value("dash", std::string("solid"));
  const auto parsed_dash = wireDashFromString(dash);
  if (!parsed_dash.has_value()) {
    return makeError(ErrorCode::SchemaViolation, "Unknown wire dash pattern", path + "/dash");
  }
  style.dash = *parsed_dash;
  style.unknown_fields = captureUnknown(value, kStyleFields);
  return style;
}

[[nodiscard]] Result<json> styleJson(const WireStyle& style) {
  json result = json::object();
  const auto merged = mergeUnknown(result, style.unknown_fields, "style");
  if (!merged) {
    return merged.error();
  }
  result["color"] = style.color;
  result["thickness"] = style.thickness;
  result["dash"] = std::string(toString(style.dash));
  return result;
}

[[nodiscard]] Result<WireEndpoint> parseEndpoint(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a wire endpoint object", path);
  }
  WireEndpoint endpoint;
  const auto component_id =
      parseId<ComponentId>(value.at("component"), kComponentPrefix, path + "/component");
  if (!component_id) {
    return component_id.error();
  }
  const auto pin_id = parseId<PinId>(value.at("pin"), kPinPrefix, path + "/pin");
  if (!pin_id) {
    return pin_id.error();
  }
  endpoint.component = component_id.value();
  endpoint.pin = pin_id.value();
  endpoint.unknown_fields = captureUnknown(value, kEndpointFields);
  return endpoint;
}

[[nodiscard]] Result<json> endpointJson(const WireEndpoint& endpoint) {
  json result = json::object();
  const auto merged = mergeUnknown(result, endpoint.unknown_fields, "endpoint");
  if (!merged) {
    return merged.error();
  }
  result["component"] = idToString(endpoint.component);
  result["pin"] = idToString(endpoint.pin);
  return result;
}

[[nodiscard]] Result<Component> parseComponent(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a component object", path);
  }
  Component component;
  const auto id = parseId<ComponentId>(value.at("id"), kComponentPrefix, path + "/id");
  if (!id) {
    return id.error();
  }
  component.id = id.value();
  component.type = value.at("type").get<std::string>();
  component.tag = value.at("tag").get<std::string>();
  component.description = value.value("description", std::string{});
  const auto position = parsePoint(value.at("position"), path + "/position");
  if (!position) {
    return position.error();
  }
  component.position = position.value();
  const auto size = parsePoint(value.at("size"), path + "/size");
  if (!size) {
    return size.error();
  }
  component.size = size.value();
  const int rotation = value.at("rotation").get<int>();
  switch (rotation) {
  case 0:
    component.rotation = Rotation::Deg0;
    break;
  case 90:
    component.rotation = Rotation::Deg90;
    break;
  case 180:
    component.rotation = Rotation::Deg180;
    break;
  case 270:
    component.rotation = Rotation::Deg270;
    break;
  default:
    return makeError(ErrorCode::SchemaViolation, "Invalid rotation", path + "/rotation");
  }

  for (std::size_t index = 0; index < value.at("pins").size(); ++index) {
    const json& pin_value = value.at("pins")[index];
    const std::string pin_path = path + "/pins/" + std::to_string(index);
    if (!pin_value.is_object()) {
      return makeError(ErrorCode::SchemaViolation, "Expected a pin object", pin_path);
    }
    Pin pin;
    const auto pin_id = parseId<PinId>(pin_value.at("id"), kPinPrefix, pin_path + "/id");
    if (!pin_id) {
      return pin_id.error();
    }
    pin.id = pin_id.value();
    pin.name = pin_value.at("name").get<std::string>();
    const std::string role = pin_value.at("role").get<std::string>();
    const auto parsed_role = pinRoleFromString(role);
    if (!parsed_role.has_value()) {
      return makeError(ErrorCode::SchemaViolation, "Unknown pin role: " + role, pin_path + "/role");
    }
    pin.role = *parsed_role;
    const auto offset = parsePoint(pin_value.at("offset"), pin_path + "/offset");
    if (!offset) {
      return offset.error();
    }
    pin.offset = offset.value();
    pin.required = pin_value.value("required", false);
    pin.unknown_fields = captureUnknown(pin_value, kPinFields);
    component.pins.push_back(std::move(pin));
  }

  const auto parameters = parseParameters(value.at("parameters"), path + "/parameters");
  if (!parameters) {
    return parameters.error();
  }
  component.parameters = parameters.value();
  const auto labels = parseLabels(value.at("labels"), path + "/labels");
  if (!labels) {
    return labels.error();
  }
  component.labels = labels.value();
  component.unknown_fields = captureUnknown(value, kComponentFields);
  return component;
}

[[nodiscard]] Result<json> componentJson(const Component& component) {
  json result = json::object();
  const auto merged = mergeUnknown(result, component.unknown_fields, "component");
  if (!merged) {
    return merged.error();
  }
  result["id"] = idToString(component.id);
  result["type"] = component.type;
  result["tag"] = component.tag;
  result["description"] = component.description;
  result["position"] = pointJson(component.position);
  result["size"] = sizeJson(component.size);
  result["rotation"] = rotationDegrees(component.rotation);
  result["parameters"] = parametersJson(component.parameters);
  const auto labels = labelsJson(component.labels);
  if (!labels) {
    return labels.error();
  }
  result["labels"] = labels.value();

  json pins = json::array();
  for (const Pin& pin : component.pins) {
    json pin_json = json::object();
    const auto pin_merged = mergeUnknown(pin_json, pin.unknown_fields, "pin");
    if (!pin_merged) {
      return pin_merged.error();
    }
    pin_json["id"] = idToString(pin.id);
    pin_json["name"] = pin.name;
    pin_json["role"] = std::string(toString(pin.role));
    pin_json["offset"] = pointJson(pin.offset);
    pin_json["required"] = pin.required;
    pins.push_back(std::move(pin_json));
  }
  result["pins"] = std::move(pins);
  return result;
}

[[nodiscard]] Result<Wire> parseWire(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a wire object", path);
  }
  Wire wire;
  const auto id = parseId<WireId>(value.at("id"), kWirePrefix, path + "/id");
  if (!id) {
    return id.error();
  }
  wire.id = id.value();
  const auto from = parseEndpoint(value.at("from"), path + "/from");
  if (!from) {
    return from.error();
  }
  wire.from = from.value();
  const auto to = parseEndpoint(value.at("to"), path + "/to");
  if (!to) {
    return to.error();
  }
  wire.to = to.value();
  if (!value.at("route").is_array()) {
    return makeError(ErrorCode::SchemaViolation, "Expected a route array", path + "/route");
  }
  for (std::size_t index = 0; index < value.at("route").size(); ++index) {
    const auto point =
        parsePoint(value.at("route")[index], path + "/route/" + std::to_string(index));
    if (!point) {
      return point.error();
    }
    wire.route.push_back(point.value());
  }
  const auto style = parseStyle(value.at("style"), path + "/style");
  if (!style) {
    return style.error();
  }
  wire.style = style.value();
  wire.allow_self_loop = value.value("allow_self_loop", false);
  wire.unknown_fields = captureUnknown(value, kWireFields);
  return wire;
}

[[nodiscard]] Result<json> wireJson(const Wire& wire) {
  json result = json::object();
  const auto merged = mergeUnknown(result, wire.unknown_fields, "wire");
  if (!merged) {
    return merged.error();
  }
  result["id"] = idToString(wire.id);
  const auto from = endpointJson(wire.from);
  if (!from) {
    return from.error();
  }
  result["from"] = from.value();
  const auto to = endpointJson(wire.to);
  if (!to) {
    return to.error();
  }
  result["to"] = to.value();
  json route = json::array();
  for (const Vec2& point : wire.route) {
    route.push_back(pointJson(point));
  }
  result["route"] = std::move(route);
  const auto style = styleJson(wire.style);
  if (!style) {
    return style.error();
  }
  result["style"] = style.value();
  result["allow_self_loop"] = wire.allow_self_loop;
  return result;
}

[[nodiscard]] Result<Annotation> parseAnnotation(const json& value, const std::string& path) {
  if (!value.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Expected an annotation object", path);
  }
  Annotation annotation;
  const auto id = parseId<AnnotationId>(value.at("id"), kAnnotationPrefix, path + "/id");
  if (!id) {
    return id.error();
  }
  annotation.id = id.value();
  const std::string kind = value.at("kind").get<std::string>();
  const auto parsed_kind = annotationKindFromString(kind);
  if (!parsed_kind.has_value()) {
    return makeError(ErrorCode::SchemaViolation, "Unknown annotation kind", path + "/kind");
  }
  annotation.kind = *parsed_kind;
  const auto position = parsePoint(value.at("position"), path + "/position");
  if (!position) {
    return position.error();
  }
  annotation.position = position.value();
  const auto size = parsePoint(value.at("size"), path + "/size");
  if (!size) {
    return size.error();
  }
  annotation.size = size.value();
  annotation.text = value.at("text").get<std::string>();
  annotation.font_size = value.at("font_size").get<double>();
  annotation.color = value.at("color").get<std::string>();
  annotation.unknown_fields = captureUnknown(value, kAnnotationFields);
  return annotation;
}

[[nodiscard]] Result<json> annotationJson(const Annotation& annotation) {
  json result = json::object();
  const auto merged = mergeUnknown(result, annotation.unknown_fields, "annotation");
  if (!merged) {
    return merged.error();
  }
  result["id"] = idToString(annotation.id);
  result["kind"] = std::string(toString(annotation.kind));
  result["position"] = pointJson(annotation.position);
  result["size"] = sizeJson(annotation.size);
  result["text"] = annotation.text;
  result["font_size"] = annotation.font_size;
  result["color"] = annotation.color;
  return result;
}

[[nodiscard]] Result<Diagram> loadDocument(const json& document) {
  Diagram diagram;
  if (document.contains("metadata")) {
    const json& metadata = document.at("metadata");
    diagram.metadata.title = metadata.value("title", std::string{});
    diagram.metadata.author = metadata.value("author", std::string{});
    diagram.metadata.created_utc = metadata.value("created_utc", std::string{});
    diagram.metadata.modified_utc = metadata.value("modified_utc", std::string{});
    diagram.metadata.revision = metadata.value("revision", std::string("A"));
    diagram.metadata.unknown_fields = captureUnknown(metadata, kMetadataFields);
  }
  diagram.unknown_fields = captureUnknown(document, kRootFields);

  const json& sheets = document.at("sheets");
  for (std::size_t sheet_index = 0; sheet_index < sheets.size(); ++sheet_index) {
    const std::string sheet_path = "/sheets/" + std::to_string(sheet_index);
    const json& sheet_value = sheets[sheet_index];
    if (!sheet_value.is_object()) {
      return makeError(ErrorCode::SchemaViolation, "Expected a sheet object", sheet_path);
    }
    Sheet sheet;
    const auto sheet_id = parseId<SheetId>(sheet_value.at("id"), kSheetPrefix, sheet_path + "/id");
    if (!sheet_id) {
      return sheet_id.error();
    }
    sheet.id = sheet_id.value();
    sheet.name = sheet_value.at("name").get<std::string>();
    const auto paper = paperSizeFromString(sheet_value.at("paper").get<std::string>());
    if (!paper.has_value()) {
      return makeError(ErrorCode::SchemaViolation, "Unknown paper size", sheet_path + "/paper");
    }
    sheet.setup.paper = *paper;
    const auto orientation =
        orientationFromString(sheet_value.at("orientation").get<std::string>());
    if (!orientation.has_value()) {
      return makeError(
          ErrorCode::SchemaViolation, "Unknown orientation", sheet_path + "/orientation");
    }
    sheet.setup.orientation = *orientation;
    sheet.setup.margin = sheet_value.value("margin", 10.0);
    sheet.setup.show_frame = sheet_value.value("show_frame", true);
    sheet.unknown_fields = captureUnknown(sheet_value, kSheetFields);
    const auto added_sheet = diagram.addSheet(std::move(sheet));
    if (!added_sheet) {
      return makeError(added_sheet.error().code, added_sheet.error().message, sheet_path);
    }

    if (!sheet_value.at("components").is_array()) {
      return makeError(
          ErrorCode::SchemaViolation, "Expected a components array", sheet_path + "/components");
    }
    for (std::size_t index = 0; index < sheet_value.at("components").size(); ++index) {
      const std::string path = sheet_path + "/components/" + std::to_string(index);
      const auto component = parseComponent(sheet_value.at("components")[index], path);
      if (!component) {
        return component.error();
      }
      const auto added = diagram.addComponent(sheet_id.value(), component.value());
      if (!added) {
        return makeError(added.error().code, added.error().message, path);
      }
    }

    if (!sheet_value.at("wires").is_array()) {
      return makeError(ErrorCode::SchemaViolation, "Expected a wires array", sheet_path + "/wires");
    }
    for (std::size_t index = 0; index < sheet_value.at("wires").size(); ++index) {
      const std::string path = sheet_path + "/wires/" + std::to_string(index);
      const auto wire = parseWire(sheet_value.at("wires")[index], path);
      if (!wire) {
        return wire.error();
      }
      const auto added = diagram.addWire(sheet_id.value(), wire.value());
      if (!added) {
        return makeError(added.error().code, added.error().message, path);
      }
    }

    if (!sheet_value.at("annotations").is_array()) {
      return makeError(
          ErrorCode::SchemaViolation, "Expected an annotations array", sheet_path + "/annotations");
    }
    for (std::size_t index = 0; index < sheet_value.at("annotations").size(); ++index) {
      const std::string path = sheet_path + "/annotations/" + std::to_string(index);
      const auto annotation = parseAnnotation(sheet_value.at("annotations")[index], path);
      if (!annotation) {
        return annotation.error();
      }
      const auto added = diagram.addAnnotation(sheet_id.value(), annotation.value());
      if (!added) {
        return makeError(added.error().code, added.error().message, path);
      }
    }
  }

  diagram.rebuildIdAllocator();
  return diagram;
}

[[nodiscard]] Result<json> documentJson(const Diagram& diagram) {
  json document = json::object();
  const auto merged = mergeUnknown(document, diagram.unknown_fields, "document");
  if (!merged) {
    return merged.error();
  }
  document["format"] = std::string(kFormatName);
  document["format_version"] = kFormatVersion;
  document["app_version"] = std::string(kAppVersion);

  json metadata = json::object();
  const auto metadata_merged = mergeUnknown(metadata, diagram.metadata.unknown_fields, "metadata");
  if (!metadata_merged) {
    return metadata_merged.error();
  }
  metadata["title"] = diagram.metadata.title;
  metadata["author"] = diagram.metadata.author;
  metadata["created_utc"] = diagram.metadata.created_utc;
  metadata["modified_utc"] = diagram.metadata.modified_utc;
  metadata["revision"] = diagram.metadata.revision;
  document["metadata"] = std::move(metadata);

  json sheets = json::array();
  std::map<std::string, int> library_refs;
  for (const Sheet& sheet : diagram.sheets) {
    json sheet_json = json::object();
    const auto sheet_merged = mergeUnknown(sheet_json, sheet.unknown_fields, "sheet");
    if (!sheet_merged) {
      return sheet_merged.error();
    }
    sheet_json["id"] = idToString(sheet.id);
    sheet_json["name"] = sheet.name;
    sheet_json["paper"] = std::string(toString(sheet.setup.paper));
    sheet_json["orientation"] = std::string(toString(sheet.setup.orientation));
    sheet_json["margin"] = sheet.setup.margin;
    sheet_json["show_frame"] = sheet.setup.show_frame;

    json components = json::array();
    for (const Component& component : sheet.components) {
      const auto value = componentJson(component);
      if (!value) {
        return value.error();
      }
      components.push_back(value.value());
      library_refs[component.type] = 1;
    }
    sheet_json["components"] = std::move(components);

    json wires = json::array();
    for (const Wire& wire : sheet.wires) {
      const auto value = wireJson(wire);
      if (!value) {
        return value.error();
      }
      wires.push_back(value.value());
    }
    sheet_json["wires"] = std::move(wires);

    json annotations = json::array();
    for (const Annotation& annotation : sheet.annotations) {
      const auto value = annotationJson(annotation);
      if (!value) {
        return value.error();
      }
      annotations.push_back(value.value());
    }
    sheet_json["annotations"] = std::move(annotations);
    sheets.push_back(std::move(sheet_json));
  }
  document["sheets"] = std::move(sheets);

  json refs = json::array();
  for (const auto& [type, version] : library_refs) {
    refs.push_back(json{{"type", type}, {"symbol_version", version}});
  }
  document["library_refs"] = std::move(refs);
  return document;
}

} // namespace

std::string idToString(ComponentId id) { return formatId(kComponentPrefix, id); }
std::string idToString(PinId id) { return formatId(kPinPrefix, id); }
std::string idToString(WireId id) { return formatId(kWirePrefix, id); }
std::string idToString(SheetId id) { return formatId(kSheetPrefix, id); }
std::string idToString(AnnotationId id) { return formatId(kAnnotationPrefix, id); }

Result<nlohmann::json> toJson(const Diagram& diagram) { return documentJson(diagram); }

Result<nlohmann::json> toJson(const Diagram& diagram, int indent) {
  (void)indent;
  return documentJson(diagram);
}

Result<std::string> toJsonString(const Diagram& diagram) {
  auto result = documentJson(diagram);
  if (!result) {
    return result.error();
  }
  return result.value().dump(2) + "\n";
}

Result<std::string> toJsonString(const Diagram& diagram, int indent) {
  auto result = documentJson(diagram);
  if (!result) {
    return result.error();
  }
  return result.value().dump(indent) + "\n";
}

Result<Diagram> fromJson(const nlohmann::json& document) {
  if (!document.is_object()) {
    return makeError(ErrorCode::SchemaViolation, "Project root must be a JSON object");
  }
  if (!document.contains("format") || !document.at("format").is_string() ||
      document.at("format").get<std::string>() != kFormatName) {
    return makeError(ErrorCode::SchemaViolation, "Not a .dcj project: expected format \"dcj\"");
  }
  if (!document.contains("format_version") || !document.at("format_version").is_number_integer()) {
    return makeError(ErrorCode::SchemaViolation, "Missing or invalid format_version");
  }
  const std::int64_t version = document.at("format_version").get<std::int64_t>();
  if (version > kFormatVersion) {
    return makeError(ErrorCode::UnsupportedVersion,
                     "This project was saved with a newer format version; please update FioVivo",
                     "format_version=" + std::to_string(version));
  }
  if (version < 1) {
    return makeError(ErrorCode::UnsupportedVersion,
                     "Unsupported project format version",
                     "format_version=" + std::to_string(version));
  }

  const auto validation = validateAgainstSchema(document, bundledSchema());
  if (!validation) {
    return validation.error();
  }

  try {
    return loadDocument(document);
  } catch (const nlohmann::json::exception& error) {
    return makeError(ErrorCode::SchemaViolation, "Invalid project structure", error.what());
  }
}

Result<Diagram> fromJsonString(std::string_view text) {
  std::string_view payload = text;
  if (payload.size() >= 3 && static_cast<unsigned char>(payload[0]) == 0xEF &&
      static_cast<unsigned char>(payload[1]) == 0xBB &&
      static_cast<unsigned char>(payload[2]) == 0xBF) {
    payload.remove_prefix(3);
  }
  try {
    const nlohmann::json document = nlohmann::json::parse(payload.begin(), payload.end());
    return fromJson(document);
  } catch (const nlohmann::json::parse_error& error) {
    return makeError(ErrorCode::ParseError,
                     "Could not parse project JSON",
                     "byte " + std::to_string(error.byte) + ": " + error.what());
  }
}

Result<Diagram> loadFile(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return makeError(ErrorCode::IoError, "Could not open project file", path.string());
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  if (input.bad()) {
    return makeError(ErrorCode::IoError, "Could not read project file", path.string());
  }
  return fromJsonString(buffer.str());
}

Result<void> saveFile(const Diagram& diagram, const std::filesystem::path& path) {
  auto text = toJsonString(diagram);
  if (!text) {
    return text.error();
  }
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    return makeError(ErrorCode::IoError, "Could not open project file for writing", path.string());
  }
  output.write(text.value().data(), static_cast<std::streamsize>(text.value().size()));
  if (!output) {
    return makeError(ErrorCode::IoError, "Could not write project file", path.string());
  }
  return {};
}

} // namespace fiovivo::io
