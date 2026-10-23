#pragma once

#include "core/model.hpp"
#include "core/result.hpp"

#include <string>

namespace fiovivo::io {

// UTF-8, LF line endings, quoted fields. Deterministic: rows are grouped by component type and
// sorted by type then tag. Parameters are listed as key=value pairs in lexical key order.
[[nodiscard]] Result<std::string> bomToCsv(const Diagram& diagram);

// Human-readable, locale-independent summary of a parameter map (used by the BOM and the UI).
[[nodiscard]] std::string parameterSummary(const ParameterMap& parameters);

} // namespace fiovivo::io
