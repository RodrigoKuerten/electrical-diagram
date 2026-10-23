#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace fiovivo {

struct ComponentIdTag;
struct PinIdTag;
struct WireIdTag;
struct SheetIdTag;
struct AnnotationIdTag;

template <typename Tag> class StrongId {
public:
  using Value = std::uint64_t;

  constexpr StrongId() = default;
  explicit constexpr StrongId(Value value) : value_(value) {}

  [[nodiscard]] constexpr Value value() const { return value_; }
  [[nodiscard]] constexpr bool valid() const { return value_ != 0; }
  [[nodiscard]] constexpr bool isValid() const { return valid(); }

  constexpr StrongId& operator++() {
    ++value_;
    return *this;
  }

  constexpr StrongId operator++(int) {
    StrongId copy(*this);
    ++value_;
    return copy;
  }

  friend constexpr auto operator<=>(const StrongId& lhs, const StrongId& rhs) = default;
  friend constexpr bool operator==(const StrongId& lhs, const StrongId& rhs) = default;

private:
  Value value_ = 0;
};

using ComponentId = StrongId<ComponentIdTag>;
using PinId = StrongId<PinIdTag>;
using WireId = StrongId<WireIdTag>;
using SheetId = StrongId<SheetIdTag>;
using AnnotationId = StrongId<AnnotationIdTag>;

struct PinRef {
  ComponentId component;
  PinId pin;

  constexpr PinRef() = default;
  constexpr PinRef(ComponentId component_id, PinId pin_id) : component(component_id), pin(pin_id) {}

  [[nodiscard]] constexpr bool valid() const { return component.valid() && pin.valid(); }

  friend constexpr auto operator<=>(const PinRef& lhs, const PinRef& rhs) = default;
  friend constexpr bool operator==(const PinRef& lhs, const PinRef& rhs) = default;
};

class IdAllocator {
public:
  [[nodiscard]] ComponentId newComponentId() { return ComponentId(next_component_++); }
  [[nodiscard]] PinId newPinId() { return PinId(next_pin_++); }
  [[nodiscard]] WireId newWireId() { return WireId(next_wire_++); }
  [[nodiscard]] SheetId newSheetId() { return SheetId(next_sheet_++); }
  [[nodiscard]] AnnotationId newAnnotationId() { return AnnotationId(next_annotation_++); }

  void observe(ComponentId id);
  void observe(PinId id);
  void observe(WireId id);
  void observe(SheetId id);
  void observe(AnnotationId id);

  [[nodiscard]] std::uint64_t nextComponentValue() const { return next_component_; }
  [[nodiscard]] std::uint64_t nextPinValue() const { return next_pin_; }
  [[nodiscard]] std::uint64_t nextWireValue() const { return next_wire_; }
  [[nodiscard]] std::uint64_t nextSheetValue() const { return next_sheet_; }
  [[nodiscard]] std::uint64_t nextAnnotationValue() const { return next_annotation_; }

  friend bool operator==(const IdAllocator&, const IdAllocator&) = default;

private:
  std::uint64_t next_component_ = 1;
  std::uint64_t next_pin_ = 1;
  std::uint64_t next_wire_ = 1;
  std::uint64_t next_sheet_ = 1;
  std::uint64_t next_annotation_ = 1;
};

} // namespace fiovivo

namespace std {

template <typename Tag> struct hash<fiovivo::StrongId<Tag>> {
  [[nodiscard]] std::size_t operator()(const fiovivo::StrongId<Tag>& id) const noexcept {
    return std::hash<std::uint64_t>{}(id.value());
  }
};

template <> struct hash<fiovivo::PinRef> {
  [[nodiscard]] std::size_t operator()(const fiovivo::PinRef& ref) const noexcept {
    const auto first = std::hash<std::uint64_t>{}(ref.component.value());
    const auto second = std::hash<std::uint64_t>{}(ref.pin.value());
    return first ^ (second + 0x9e3779b97f4a7c15ULL + (first << 6U) + (first >> 2U));
  }
};

} // namespace std
