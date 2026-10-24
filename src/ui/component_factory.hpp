#pragma once

#include "core/model.hpp"
#include "library/symbol_definition.hpp"

#include <string>
#include <vector>

namespace fiovivo::ui {

[[nodiscard]] Component
createSymbolComponent(const SymbolDefinition& definition, Diagram& diagram, Vec2 position);
[[nodiscard]] Component cloneComponent(const Component& source, Diagram& diagram, Vec2 offset);
[[nodiscard]] Annotation makeTextAnnotation(Diagram& diagram, Vec2 position, std::string text);
[[nodiscard]] Annotation makeRectangleAnnotation(Diagram& diagram, Vec2 position, Vec2 size);

} // namespace fiovivo::ui
