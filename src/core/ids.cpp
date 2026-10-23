#include "core/ids.hpp"

#include <algorithm>

namespace fiovivo {
namespace {

template <typename Id> void observeId(std::uint64_t& next, Id id) {
  if (id.valid() && id.value() >= next) {
    next = id.value() + 1;
  }
}

} // namespace

void IdAllocator::observe(ComponentId id) { observeId(next_component_, id); }
void IdAllocator::observe(PinId id) { observeId(next_pin_, id); }
void IdAllocator::observe(WireId id) { observeId(next_wire_, id); }
void IdAllocator::observe(SheetId id) { observeId(next_sheet_, id); }
void IdAllocator::observe(AnnotationId id) { observeId(next_annotation_, id); }

} // namespace fiovivo
