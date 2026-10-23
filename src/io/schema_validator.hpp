#pragma once

#include "core/result.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace fiovivo::io {

// A deliberately small JSON Schema subset validator. It supports the keywords used by
// docs/file-format.schema.json: $ref (local pointers), type, const, enum, required, properties,
// additionalProperties (boolean), items, minItems, maxItems, minLength, maxLength, pattern,
// minimum and maximum. Unknown keywords are ignored so the schema document can remain standard.
[[nodiscard]] Result<void> validateAgainstSchema(const nlohmann::json& document,
                                                 const nlohmann::json& schema);

// The schema embedded from docs/file-format.schema.json at configure time.
[[nodiscard]] const nlohmann::json& bundledSchema();

} // namespace fiovivo::io
