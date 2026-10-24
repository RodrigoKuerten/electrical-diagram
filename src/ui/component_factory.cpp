#include "ui/component_factory.hpp"

#include "core/types.hpp"

#include <utility>

namespace fiovivo::ui {

Component
createSymbolComponent(const SymbolDefinition& definition, Diagram& diagram, Vec2 position) {
  const std::string tag = diagram.uniqueTag(SymbolLibrary::defaultTagBase(definition.type));
  return instantiateSymbol(definition, diagram, position, tag);
}

Component cloneComponent(const Component& source, Diagram& diagram, Vec2 offset) {
  Component component = source;
  component.id = diagram.ids().newComponentId();
  for (Pin& pin : component.pins) {
    pin.id = diagram.ids().newPinId();
    pin.wires.clear();
  }
  component.tag = diagram.uniqueTag(source.tag);
  component.position += offset;
  return component;
}

Annotation makeTextAnnotation(Diagram& diagram, Vec2 position, std::string text) {
  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  annotation.kind = AnnotationKind::Text;
  annotation.position = position;
  annotation.size = {60.0, 8.0};
  annotation.text = std::move(text);
  return annotation;
}

Annotation makeRectangleAnnotation(Diagram& diagram, Vec2 position, Vec2 size) {
  Annotation annotation;
  annotation.id = diagram.ids().newAnnotationId();
  annotation.kind = AnnotationKind::Rectangle;
  annotation.position = position;
  annotation.size = size;
  return annotation;
}

} // namespace fiovivo::ui
