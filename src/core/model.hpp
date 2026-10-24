#pragma once

#include "core/geometry.hpp"
#include "core/ids.hpp"
#include "core/result.hpp"
#include "core/types.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fiovivo {

struct Pin {
  PinId id;
  std::string name;
  PinRole role = PinRole::Terminal;
  Vec2 offset;
  bool required = false;
  std::vector<WireId> wires;
  UnknownFields unknown_fields;

  friend bool operator==(const Pin&, const Pin&) = default;
};

struct Component {
  ComponentId id;
  std::string type;
  std::string tag;
  std::string description;
  Vec2 position;
  Vec2 size{40.0, 40.0};
  Rotation rotation = Rotation::Deg0;
  std::vector<Pin> pins;
  ParameterMap parameters;
  Labels labels;
  UnknownFields unknown_fields;

  [[nodiscard]] const Pin* findPin(PinId pin_id) const;
  [[nodiscard]] Pin* findPin(PinId pin_id);
  [[nodiscard]] const Pin* findPin(std::string_view name) const;
  [[nodiscard]] Pin* findPin(std::string_view name);
  [[nodiscard]] Vec2 pinOffset(const Pin& pin) const;
  [[nodiscard]] Vec2 pinPosition(const Pin& pin) const;
  [[nodiscard]] std::optional<Vec2> pinPosition(std::string_view name) const;
  [[nodiscard]] Rect boundingBox() const;
  [[nodiscard]] std::string labelGroup() const;
  [[nodiscard]] bool hasRole(PinRole role) const;
  [[nodiscard]] std::size_t pinCount() const { return pins.size(); }

  friend bool operator==(const Component&, const Component&) = default;
};

struct WireEndpoint {
  ComponentId component;
  PinId pin;
  UnknownFields unknown_fields;

  WireEndpoint() = default;
  WireEndpoint(ComponentId component_id, PinId pin_id) : component(component_id), pin(pin_id) {}
  explicit WireEndpoint(const PinRef& ref) : component(ref.component), pin(ref.pin) {}

  [[nodiscard]] PinRef toPinRef() const { return {component, pin}; }
  friend bool operator==(const WireEndpoint&, const WireEndpoint&) = default;
};

struct Wire {
  WireId id;
  WireEndpoint from;
  WireEndpoint to;
  std::vector<Vec2> route;
  WireStyle style;
  bool allow_self_loop = false;
  UnknownFields unknown_fields;

  friend bool operator==(const Wire&, const Wire&) = default;
};

struct Annotation {
  AnnotationId id;
  AnnotationKind kind = AnnotationKind::Text;
  Vec2 position;
  Vec2 size{40.0, 10.0};
  std::string text;
  double font_size = 3.5;
  // Empty means "use the theme's sheet.text token".
  std::string color;
  UnknownFields unknown_fields;

  friend bool operator==(const Annotation&, const Annotation&) = default;
};

class Sheet {
public:
  SheetId id;
  std::string name;
  PaperSetup setup;
  std::vector<Component> components;
  std::vector<Wire> wires;
  std::vector<Annotation> annotations;
  UnknownFields unknown_fields;

  [[nodiscard]] const Component* findComponent(ComponentId component_id) const;
  [[nodiscard]] Component* findComponent(ComponentId component_id);
  [[nodiscard]] const Component* findComponentByTag(std::string_view tag) const;
  [[nodiscard]] Component* findComponentByTag(std::string_view tag);
  [[nodiscard]] const Wire* findWire(WireId wire_id) const;
  [[nodiscard]] Wire* findWire(WireId wire_id);
  [[nodiscard]] const Pin* findPin(PinRef ref) const;
  [[nodiscard]] Pin* findPin(PinRef ref);
  [[nodiscard]] std::optional<Vec2> pinPosition(PinRef ref) const;
  [[nodiscard]] std::vector<WireId> wiresAtPin(PinRef ref) const;
  [[nodiscard]] Rect bounds() const;

  Result<ComponentId> addComponent(Component component);
  Result<WireId> addWire(Wire wire);
  Result<AnnotationId> addAnnotation(Annotation annotation);
  bool removeComponent(ComponentId component_id);
  bool removeWire(WireId wire_id);
  bool removeAnnotation(AnnotationId annotation_id);
  void clear();

  [[nodiscard]] bool hasComponent(ComponentId component_id) const;
  [[nodiscard]] bool hasWire(WireId wire_id) const;

  friend bool operator==(const Sheet&, const Sheet&) = default;
};

class Diagram {
public:
  Metadata metadata;
  std::vector<Sheet> sheets;
  UnknownFields unknown_fields;

  [[nodiscard]] Sheet* findSheet(SheetId sheet_id);
  [[nodiscard]] const Sheet* findSheet(SheetId sheet_id) const;
  [[nodiscard]] Sheet* findSheetForComponent(ComponentId component_id);
  [[nodiscard]] const Sheet* findSheetForComponent(ComponentId component_id) const;
  [[nodiscard]] Component* findComponent(ComponentId component_id);
  [[nodiscard]] const Component* findComponent(ComponentId component_id) const;
  [[nodiscard]] Component* findComponentByTag(std::string_view tag);
  [[nodiscard]] const Component* findComponentByTag(std::string_view tag) const;
  [[nodiscard]] Pin* findPin(PinRef ref);
  [[nodiscard]] const Pin* findPin(PinRef ref) const;
  [[nodiscard]] Wire* findWire(WireId wire_id);
  [[nodiscard]] const Wire* findWire(WireId wire_id) const;

  [[nodiscard]] bool isTagUsed(std::string_view tag, ComponentId ignore = {}) const;
  [[nodiscard]] std::string uniqueTag(std::string_view base) const;
  [[nodiscard]] std::size_t componentCount() const;
  [[nodiscard]] std::size_t wireCount() const;

  Result<SheetId> addSheet(Sheet sheet);
  Result<ComponentId> addComponent(SheetId sheet_id, Component component);
  Result<WireId> addWire(SheetId sheet_id, Wire wire);
  Result<WireId> createWire(SheetId sheet_id,
                            PinRef from,
                            PinRef to,
                            const std::vector<Rect>& obstacles = {},
                            WireStyle style = {},
                            bool allow_self_loop = false);
  Result<AnnotationId> addAnnotation(SheetId sheet_id, Annotation annotation);
  bool removeSheet(SheetId sheet_id);
  bool removeComponent(ComponentId component_id);
  bool removeWire(WireId wire_id);

  [[nodiscard]] IdAllocator& ids() { return ids_; }
  [[nodiscard]] const IdAllocator& ids() const { return ids_; }

  void rebuildIdAllocator();

  friend bool operator==(const Diagram&, const Diagram&) = default;

private:
  IdAllocator ids_;
};

[[nodiscard]] std::string tagBaseName(std::string_view tag);
[[nodiscard]] std::string endpointName(const Sheet& sheet, const WireEndpoint& endpoint);

} // namespace fiovivo
