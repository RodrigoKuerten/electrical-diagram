#include "core/model.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <set>
#include <utility>

namespace fiovivo {
namespace {

[[nodiscard]] bool pinHasWire(const Pin& pin, WireId id) {
  return std::find(pin.wires.begin(), pin.wires.end(), id) != pin.wires.end();
}

[[nodiscard]] Pin* mutablePin(Sheet& sheet, const WireEndpoint& endpoint) {
  Component* component = sheet.findComponent(endpoint.component);
  return component == nullptr ? nullptr : component->findPin(endpoint.pin);
}

} // namespace

const Pin* Component::findPin(PinId pin_id) const {
  const auto iterator = std::find_if(pins.begin(), pins.end(),
                                     [pin_id](const Pin& pin) { return pin.id == pin_id; });
  return iterator == pins.end() ? nullptr : &*iterator;
}

Pin* Component::findPin(PinId pin_id) {
  const auto iterator = std::find_if(pins.begin(), pins.end(),
                                     [pin_id](const Pin& pin) { return pin.id == pin_id; });
  return iterator == pins.end() ? nullptr : &*iterator;
}

const Pin* Component::findPin(std::string_view name) const {
  const auto iterator =
      std::find_if(pins.begin(), pins.end(), [name](const Pin& pin) { return pin.name == name; });
  return iterator == pins.end() ? nullptr : &*iterator;
}

Pin* Component::findPin(std::string_view name) {
  const auto iterator =
      std::find_if(pins.begin(), pins.end(), [name](const Pin& pin) { return pin.name == name; });
  return iterator == pins.end() ? nullptr : &*iterator;
}

Vec2 Component::pinOffset(const Pin& pin) const {
  return rotateAround(pin.offset, size / 2.0, rotation);
}

Vec2 Component::pinPosition(const Pin& pin) const { return position + pinOffset(pin); }

std::optional<Vec2> Component::pinPosition(std::string_view name) const {
  const Pin* pin = findPin(name);
  if (pin == nullptr) {
    return std::nullopt;
  }
  return pinPosition(*pin);
}

Rect Component::boundingBox() const { return rotatedBounds(position, size, rotation); }

std::string Component::labelGroup() const {
  const std::string explicit_group = parameterString(parameters, "label_group");
  if (!explicit_group.empty()) {
    return explicit_group;
  }
  return tagBaseName(tag);
}

bool Component::hasRole(PinRole role) const {
  return std::any_of(pins.begin(), pins.end(),
                     [role](const Pin& pin) { return pin.role == role; });
}

const Component* Sheet::findComponent(ComponentId component_id) const {
  const auto iterator =
      std::find_if(components.begin(), components.end(), [component_id](const Component& item) {
        return item.id == component_id;
      });
  return iterator == components.end() ? nullptr : &*iterator;
}

Component* Sheet::findComponent(ComponentId component_id) {
  const auto iterator =
      std::find_if(components.begin(), components.end(), [component_id](const Component& item) {
        return item.id == component_id;
      });
  return iterator == components.end() ? nullptr : &*iterator;
}

const Component* Sheet::findComponentByTag(std::string_view tag) const {
  const auto iterator =
      std::find_if(components.begin(), components.end(),
                   [tag](const Component& item) { return item.tag == tag; });
  return iterator == components.end() ? nullptr : &*iterator;
}

Component* Sheet::findComponentByTag(std::string_view tag) {
  const auto iterator =
      std::find_if(components.begin(), components.end(),
                   [tag](const Component& item) { return item.tag == tag; });
  return iterator == components.end() ? nullptr : &*iterator;
}

const Wire* Sheet::findWire(WireId wire_id) const {
  const auto iterator =
      std::find_if(wires.begin(), wires.end(),
                   [wire_id](const Wire& wire) { return wire.id == wire_id; });
  return iterator == wires.end() ? nullptr : &*iterator;
}

Wire* Sheet::findWire(WireId wire_id) {
  const auto iterator =
      std::find_if(wires.begin(), wires.end(),
                   [wire_id](const Wire& wire) { return wire.id == wire_id; });
  return iterator == wires.end() ? nullptr : &*iterator;
}

const Pin* Sheet::findPin(PinRef ref) const {
  const Component* component = findComponent(ref.component);
  return component == nullptr ? nullptr : component->findPin(ref.pin);
}

Pin* Sheet::findPin(PinRef ref) {
  Component* component = findComponent(ref.component);
  return component == nullptr ? nullptr : component->findPin(ref.pin);
}

std::optional<Vec2> Sheet::pinPosition(PinRef ref) const {
  const Pin* pin = findPin(ref);
  if (pin == nullptr) {
    return std::nullopt;
  }
  return findComponent(ref.component)->pinPosition(*pin);
}

std::vector<WireId> Sheet::wiresAtPin(PinRef ref) const {
  const Pin* pin = findPin(ref);
  return pin == nullptr ? std::vector<WireId>{} : pin->wires;
}

Rect Sheet::bounds() const {
  if (components.empty() && annotations.empty()) {
    return {0.0, 0.0, 0.0, 0.0};
  }
  std::vector<Vec2> points;
  points.reserve(components.size() * 2 + annotations.size() * 2);
  for (const Component& component : components) {
    const Rect rect = component.boundingBox();
    points.push_back(rect.topLeft());
    points.push_back({rect.right(), rect.bottom()});
  }
  for (const Annotation& annotation : annotations) {
    points.push_back(annotation.position);
    points.push_back(annotation.position + annotation.size);
  }
  return boundingRect(points);
}

Result<ComponentId> Sheet::addComponent(Component component) {
  if (!component.id.valid()) {
    return makeError(ErrorCode::InvalidArgument, "Component id must be valid");
  }
  if (component.type.empty()) {
    return makeError(ErrorCode::InvalidArgument, "Component type must not be empty");
  }
  if (component.tag.empty()) {
    return makeError(ErrorCode::InvalidArgument, "Component tag must not be empty");
  }
  if (findComponent(component.id) != nullptr) {
    return makeError(ErrorCode::DuplicateId, "Duplicate component id", component.tag);
  }
  std::set<PinId> pin_ids;
  std::set<std::string> pin_names;
  for (const Pin& pin : component.pins) {
    if (!pin.id.valid()) {
      return makeError(ErrorCode::InvalidArgument, "Pin id must be valid", component.tag);
    }
    if (pin.name.empty()) {
      return makeError(ErrorCode::InvalidArgument, "Pin name must not be empty", component.tag);
    }
    if (!pin_ids.insert(pin.id).second) {
      return makeError(ErrorCode::DuplicateId, "Duplicate pin id in component", component.tag);
    }
    if (!pin_names.insert(pin.name).second) {
      return makeError(ErrorCode::InvalidArgument, "Duplicate pin name in component", pin.name);
    }
  }
  const ComponentId component_id = component.id;
  components.push_back(std::move(component));
  return component_id;
}

Result<WireId> Sheet::addWire(Wire wire) {
  if (!wire.id.valid()) {
    return makeError(ErrorCode::InvalidArgument, "Wire id must be valid");
  }
  if (findWire(wire.id) != nullptr) {
    return makeError(ErrorCode::DuplicateId, "Duplicate wire id");
  }
  if (wire.from.pin == wire.to.pin) {
    return makeError(ErrorCode::SamePin, "A wire must connect two distinct pins");
  }
  Component* from_component = findComponent(wire.from.component);
  Component* to_component = findComponent(wire.to.component);
  if (from_component == nullptr || to_component == nullptr) {
    return makeError(ErrorCode::InvalidReference, "Wire endpoint component does not exist");
  }
  Pin* from_pin = from_component->findPin(wire.from.pin);
  Pin* to_pin = to_component->findPin(wire.to.pin);
  if (from_pin == nullptr || to_pin == nullptr) {
    return makeError(ErrorCode::InvalidReference, "Wire endpoint pin does not exist");
  }
  if (wire.from.component == wire.to.component && !wire.allow_self_loop) {
    return makeError(ErrorCode::SelfLoopNotAllowed,
                     "Self-loop wires require allow_self_loop", from_component->tag);
  }
  for (const WireId existing_id : from_pin->wires) {
    const Wire* existing = findWire(existing_id);
    if (existing == nullptr) {
      continue;
    }
    const bool same_direction =
        existing->from == wire.from && existing->to == wire.to;
    const bool opposite_direction =
        existing->from == wire.to && existing->to == wire.from;
    if (same_direction || opposite_direction) {
      return makeError(ErrorCode::DuplicateWire, "These pins are already connected",
                       from_component->tag);
    }
  }
  if (wire.route.size() < 2) {
    wire.route = {from_component->pinPosition(*from_pin), to_component->pinPosition(*to_pin)};
  }
  if (pinHasWire(*from_pin, wire.id) || pinHasWire(*to_pin, wire.id)) {
    return makeError(ErrorCode::DuplicateId, "Wire already registered on a pin");
  }
  const WireId wire_id = wire.id;
  from_pin->wires.push_back(wire_id);
  to_pin->wires.push_back(wire_id);
  wires.push_back(std::move(wire));
  return wire_id;
}

Result<AnnotationId> Sheet::addAnnotation(Annotation annotation) {
  if (!annotation.id.valid()) {
    return makeError(ErrorCode::InvalidArgument, "Annotation id must be valid");
  }
  const auto iterator =
      std::find_if(annotations.begin(), annotations.end(), [&annotation](const Annotation& item) {
        return item.id == annotation.id;
      });
  if (iterator != annotations.end()) {
    return makeError(ErrorCode::DuplicateId, "Duplicate annotation id");
  }
  const AnnotationId annotation_id = annotation.id;
  annotations.push_back(std::move(annotation));
  return annotation_id;
}

bool Sheet::removeComponent(ComponentId component_id) {
  std::set<WireId> affected;
  for (const Wire& wire : wires) {
    if (wire.from.component == component_id || wire.to.component == component_id) {
      affected.insert(wire.id);
    }
  }
  for (const WireId wire_id : affected) {
    removeWire(wire_id);
  }
  const auto iterator =
      std::remove_if(components.begin(), components.end(),
                     [component_id](const Component& item) { return item.id == component_id; });
  if (iterator == components.end()) {
    return false;
  }
  components.erase(iterator, components.end());
  return true;
}

bool Sheet::removeWire(WireId wire_id) {
  const auto iterator = std::find_if(wires.begin(), wires.end(),
                                     [wire_id](const Wire& wire) { return wire.id == wire_id; });
  if (iterator == wires.end()) {
    return false;
  }
  if (Pin* from_pin = mutablePin(*this, iterator->from)) {
    from_pin->wires.erase(std::remove(from_pin->wires.begin(), from_pin->wires.end(), wire_id),
                          from_pin->wires.end());
  }
  if (Pin* to_pin = mutablePin(*this, iterator->to)) {
    to_pin->wires.erase(std::remove(to_pin->wires.begin(), to_pin->wires.end(), wire_id),
                        to_pin->wires.end());
  }
  wires.erase(iterator);
  return true;
}

bool Sheet::removeAnnotation(AnnotationId annotation_id) {
  const auto iterator =
      std::remove_if(annotations.begin(), annotations.end(), [annotation_id](const Annotation& a) {
        return a.id == annotation_id;
      });
  if (iterator == annotations.end()) {
    return false;
  }
  annotations.erase(iterator, annotations.end());
  return true;
}

void Sheet::clear() {
  components.clear();
  wires.clear();
  annotations.clear();
}

bool Sheet::hasComponent(ComponentId component_id) const { return findComponent(component_id) != nullptr; }

bool Sheet::hasWire(WireId wire_id) const { return findWire(wire_id) != nullptr; }

Sheet* Diagram::findSheet(SheetId sheet_id) {
  const auto iterator =
      std::find_if(sheets.begin(), sheets.end(),
                   [sheet_id](const Sheet& sheet) { return sheet.id == sheet_id; });
  return iterator == sheets.end() ? nullptr : &*iterator;
}

const Sheet* Diagram::findSheet(SheetId sheet_id) const {
  const auto iterator =
      std::find_if(sheets.begin(), sheets.end(),
                   [sheet_id](const Sheet& sheet) { return sheet.id == sheet_id; });
  return iterator == sheets.end() ? nullptr : &*iterator;
}

Sheet* Diagram::findSheetForComponent(ComponentId component_id) {
  for (Sheet& sheet : sheets) {
    if (sheet.findComponent(component_id) != nullptr) {
      return &sheet;
    }
  }
  return nullptr;
}

const Sheet* Diagram::findSheetForComponent(ComponentId component_id) const {
  for (const Sheet& sheet : sheets) {
    if (sheet.findComponent(component_id) != nullptr) {
      return &sheet;
    }
  }
  return nullptr;
}

Component* Diagram::findComponent(ComponentId component_id) {
  for (Sheet& sheet : sheets) {
    if (Component* component = sheet.findComponent(component_id)) {
      return component;
    }
  }
  return nullptr;
}

const Component* Diagram::findComponent(ComponentId component_id) const {
  for (const Sheet& sheet : sheets) {
    if (const Component* component = sheet.findComponent(component_id)) {
      return component;
    }
  }
  return nullptr;
}

Component* Diagram::findComponentByTag(std::string_view tag) {
  for (Sheet& sheet : sheets) {
    if (Component* component = sheet.findComponentByTag(tag)) {
      return component;
    }
  }
  return nullptr;
}

const Component* Diagram::findComponentByTag(std::string_view tag) const {
  for (const Sheet& sheet : sheets) {
    if (const Component* component = sheet.findComponentByTag(tag)) {
      return component;
    }
  }
  return nullptr;
}

Pin* Diagram::findPin(PinRef ref) {
  Sheet* sheet = findSheetForComponent(ref.component);
  return sheet == nullptr ? nullptr : sheet->findPin(ref);
}

const Pin* Diagram::findPin(PinRef ref) const {
  const Sheet* sheet = findSheetForComponent(ref.component);
  return sheet == nullptr ? nullptr : sheet->findPin(ref);
}

Wire* Diagram::findWire(WireId wire_id) {
  for (Sheet& sheet : sheets) {
    if (Wire* wire = sheet.findWire(wire_id)) {
      return wire;
    }
  }
  return nullptr;
}

const Wire* Diagram::findWire(WireId wire_id) const {
  for (const Sheet& sheet : sheets) {
    if (const Wire* wire = sheet.findWire(wire_id)) {
      return wire;
    }
  }
  return nullptr;
}

bool Diagram::isTagUsed(std::string_view tag, ComponentId ignore) const {
  for (const Sheet& sheet : sheets) {
    for (const Component& component : sheet.components) {
      if (component.id != ignore && component.tag == tag) {
        return true;
      }
    }
  }
  return false;
}

std::string Diagram::uniqueTag(std::string_view base) const {
  std::string clean(base);
  if (clean.empty()) {
    clean = "X";
  }
  if (!isTagUsed(clean)) {
    return clean;
  }
  std::size_t digit_start = clean.size();
  while (digit_start > 0 &&
         std::isdigit(static_cast<unsigned char>(clean[digit_start - 1])) != 0) {
    --digit_start;
  }
  const std::string stem = clean.substr(0, digit_start);
  const std::string numeric_suffix = clean.substr(digit_start);
  long long numeric_base = 1;
  if (!numeric_suffix.empty()) {
    const auto parsed = std::from_chars(numeric_suffix.data(),
                                        numeric_suffix.data() + numeric_suffix.size(), numeric_base);
    if (parsed.ec != std::errc{}) {
      numeric_base = 1;
    }
  }
  for (long long suffix = numeric_base + 1; suffix < numeric_base + 1000000; ++suffix) {
    const std::string candidate = stem + std::to_string(suffix);
    if (!isTagUsed(candidate)) {
      return candidate;
    }
  }
  return clean + "_copy";
}

std::size_t Diagram::componentCount() const {
  std::size_t total = 0;
  for (const Sheet& sheet : sheets) {
    total += sheet.components.size();
  }
  return total;
}

std::size_t Diagram::wireCount() const {
  std::size_t total = 0;
  for (const Sheet& sheet : sheets) {
    total += sheet.wires.size();
  }
  return total;
}

Result<SheetId> Diagram::addSheet(Sheet sheet) {
  if (!sheet.id.valid()) {
    return makeError(ErrorCode::InvalidArgument, "Sheet id must be valid");
  }
  if (findSheet(sheet.id) != nullptr) {
    return makeError(ErrorCode::DuplicateId, "Duplicate sheet id", sheet.name);
  }
  if (sheet.name.empty()) {
    return makeError(ErrorCode::InvalidArgument, "Sheet name must not be empty");
  }
  const SheetId id = sheet.id;
  sheets.push_back(std::move(sheet));
  return id;
}

Result<ComponentId> Diagram::addComponent(SheetId sheet_id, Component component) {
  Sheet* sheet = findSheet(sheet_id);
  if (sheet == nullptr) {
    return makeError(ErrorCode::NotFound, "Sheet does not exist");
  }
  if (findComponent(component.id) != nullptr) {
    return makeError(ErrorCode::DuplicateId, "Duplicate component id in diagram", component.tag);
  }
  for (const Sheet& other_sheet : sheets) {
    for (const Component& other : other_sheet.components) {
      for (const Pin& pin : component.pins) {
        if (other.findPin(pin.id) != nullptr) {
          return makeError(ErrorCode::DuplicateId, "Duplicate pin id in diagram", component.tag);
        }
      }
    }
  }
  const ComponentId component_id = component.id;
  Result<ComponentId> result = sheet->addComponent(std::move(component));
  if (!result) {
    return result;
  }
  ids_.observe(component_id);
  if (const Component* added = sheet->findComponent(component_id)) {
    for (const Pin& pin : added->pins) {
      ids_.observe(pin.id);
    }
  }
  return component_id;
}

Result<WireId> Diagram::addWire(SheetId sheet_id, Wire wire) {
  Sheet* sheet = findSheet(sheet_id);
  if (sheet == nullptr) {
    return makeError(ErrorCode::NotFound, "Sheet does not exist");
  }
  const WireId id = wire.id;
  Result<WireId> result = sheet->addWire(std::move(wire));
  if (result) {
    ids_.observe(id);
  }
  return result;
}

Result<WireId> Diagram::createWire(SheetId sheet_id, PinRef from, PinRef to,
                                   const std::vector<Rect>& obstacles, WireStyle style,
                                   bool allow_self_loop) {
  Sheet* sheet = findSheet(sheet_id);
  if (sheet == nullptr) {
    return makeError(ErrorCode::NotFound, "Sheet does not exist");
  }
  if (!from.valid() || !to.valid()) {
    return makeError(ErrorCode::InvalidArgument, "Both wire endpoints must be valid pins");
  }
  const std::optional<Vec2> from_position = sheet->pinPosition(from);
  const std::optional<Vec2> to_position = sheet->pinPosition(to);
  if (!from_position.has_value() || !to_position.has_value()) {
    return makeError(ErrorCode::InvalidReference, "Wire endpoint pin does not exist");
  }
  Wire wire;
  wire.id = ids_.newWireId();
  wire.from = WireEndpoint(from);
  wire.to = WireEndpoint(to);
  wire.style = style;
  wire.allow_self_loop = allow_self_loop;
  wire.route = orthogonalRoute(*from_position, *to_position, {}, {}, obstacles);
  return addWire(sheet_id, std::move(wire));
}

Result<AnnotationId> Diagram::addAnnotation(SheetId sheet_id, Annotation annotation) {
  Sheet* sheet = findSheet(sheet_id);
  if (sheet == nullptr) {
    return makeError(ErrorCode::NotFound, "Sheet does not exist");
  }
  const AnnotationId id = annotation.id;
  Result<AnnotationId> result = sheet->addAnnotation(std::move(annotation));
  if (result) {
    ids_.observe(id);
  }
  return result;
}

bool Diagram::removeSheet(SheetId sheet_id) {
  const auto iterator =
      std::remove_if(sheets.begin(), sheets.end(),
                     [sheet_id](const Sheet& sheet) { return sheet.id == sheet_id; });
  if (iterator == sheets.end()) {
    return false;
  }
  sheets.erase(iterator, sheets.end());
  return true;
}

bool Diagram::removeComponent(ComponentId component_id) {
  Sheet* sheet = findSheetForComponent(component_id);
  return sheet != nullptr && sheet->removeComponent(component_id);
}

bool Diagram::removeWire(WireId wire_id) {
  for (Sheet& sheet : sheets) {
    if (sheet.removeWire(wire_id)) {
      return true;
    }
  }
  return false;
}

void Diagram::rebuildIdAllocator() {
  IdAllocator allocator;
  for (const Sheet& sheet : sheets) {
    allocator.observe(sheet.id);
    for (const Component& component : sheet.components) {
      allocator.observe(component.id);
      for (const Pin& pin : component.pins) {
        allocator.observe(pin.id);
      }
    }
    for (const Wire& wire : sheet.wires) {
      allocator.observe(wire.id);
    }
    for (const Annotation& annotation : sheet.annotations) {
      allocator.observe(annotation.id);
    }
  }
  ids_ = allocator;
}

std::string tagBaseName(std::string_view tag) {
  const std::size_t separator = tag.find_last_of(":.#");
  if (separator != std::string_view::npos && separator > 0) {
    return std::string(tag.substr(0, separator));
  }
  return std::string(tag);
}

std::string endpointName(const Sheet& sheet, const WireEndpoint& endpoint) {
  const Component* component = sheet.findComponent(endpoint.component);
  if (component == nullptr) {
    return "?";
  }
  const Pin* pin = component->findPin(endpoint.pin);
  return component->tag + ":" + (pin == nullptr ? std::string("?") : pin->name);
}

} // namespace fiovivo
