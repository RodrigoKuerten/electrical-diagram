#include "io/bom.hpp"

#include <algorithm>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace fiovivo::io {
namespace {

[[nodiscard]] std::string csvField(std::string_view value) {
  std::string result;
  result.reserve(value.size() + 2);
  result.push_back('"');
  for (const char character : value) {
    if (character == '"') {
      result.push_back('"');
    }
    result.push_back(character);
  }
  result.push_back('"');
  return result;
}

[[nodiscard]] std::string numberToString(double value) {
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream << std::setprecision(12) << value;
  return stream.str();
}

[[nodiscard]] std::string valueToString(const ParameterValue& value) {
  if (const auto* boolean = std::get_if<bool>(&value)) {
    return *boolean ? "true" : "false";
  }
  if (const auto* integer = std::get_if<std::int64_t>(&value)) {
    return std::to_string(*integer);
  }
  if (const auto* number = std::get_if<double>(&value)) {
    return numberToString(*number);
  }
  if (const auto* text = std::get_if<std::string>(&value)) {
    return *text;
  }
  return {};
}

struct Row {
  std::string sheet;
  std::string tag;
  std::string type;
  std::string description;
  std::string parameters;
};

} // namespace

std::string parameterSummary(const ParameterMap& parameters) {
  std::string result;
  for (const auto& [key, value] : parameters) {
    if (!result.empty()) {
      result += "; ";
    }
    result += key;
    result += '=';
    result += valueToString(value);
  }
  return result;
}

Result<std::string> bomToCsv(const Diagram& diagram) {
  std::vector<Row> rows;
  for (const Sheet& sheet : diagram.sheets) {
    for (const Component& component : sheet.components) {
      rows.push_back(Row{sheet.name,
                         component.tag,
                         component.type,
                         component.description,
                         parameterSummary(component.parameters)});
    }
  }
  std::stable_sort(rows.begin(), rows.end(), [](const Row& left, const Row& right) {
    if (left.type != right.type) {
      return left.type < right.type;
    }
    if (left.tag != right.tag) {
      return left.tag < right.tag;
    }
    return left.sheet < right.sheet;
  });

  std::string csv;
  csv += "\"Sheet\",\"Tag\",\"Type\",\"Description\",\"Parameters\"\n";
  for (const Row& row : rows) {
    csv += csvField(row.sheet);
    csv += ',';
    csv += csvField(row.tag);
    csv += ',';
    csv += csvField(row.type);
    csv += ',';
    csv += csvField(row.description);
    csv += ',';
    csv += csvField(row.parameters);
    csv += '\n';
  }
  return csv;
}

} // namespace fiovivo::io
