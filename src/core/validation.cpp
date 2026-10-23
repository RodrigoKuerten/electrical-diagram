#include "core/validation.hpp"

#include "core/types.hpp"

#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace fiovivo {
namespace {

class DisjointSet {
public:
  int add() {
    const int index = static_cast<int>(parents_.size());
    parents_.push_back(index);
    ranks_.push_back(0);
    return index;
  }

  int find(int index) {
    if (parents_[static_cast<std::size_t>(index)] != index) {
      parents_[static_cast<std::size_t>(index)] = find(parents_[static_cast<std::size_t>(index)]);
    }
    return parents_[static_cast<std::size_t>(index)];
  }

  void unite(int left, int right) {
    int root_left = find(left);
    int root_right = find(right);
    if (root_left == root_right) {
      return;
    }
    if (ranks_[static_cast<std::size_t>(root_left)] <
        ranks_[static_cast<std::size_t>(root_right)]) {
      std::swap(root_left, root_right);
    }
    parents_[static_cast<std::size_t>(root_right)] = root_left;
    if (ranks_[static_cast<std::size_t>(root_left)] ==
        ranks_[static_cast<std::size_t>(root_right)]) {
      ++ranks_[static_cast<std::size_t>(root_left)];
    }
  }

private:
  std::vector<int> parents_;
  std::vector<int> ranks_;
};

[[nodiscard]] std::string sourceGroup(const Component& component, const Pin& pin) {
  return component.type + ":" + pin.name;
}

[[nodiscard]] std::optional<std::pair<PinRef, PinRef>> passThroughPair(const Component& component) {
  using namespace component_type;
  const auto rolePin = [&component](PinRole role) -> const Pin* {
    for (const Pin& pin : component.pins) {
      if (pin.role == role) {
        return &pin;
      }
    }
    return nullptr;
  };
  const auto byName = [&component](std::string_view name) -> const Pin* {
    return component.findPin(name);
  };

  const Pin* first = nullptr;
  const Pin* second = nullptr;

  if (component.type == kFuse || component.type == kBreaker) {
    first = byName("1");
    second = byName("2");
  } else if (component.type == kContactorNo || component.type == kContactorAuxNo ||
             component.type == kPushbuttonNo || component.type == kTimerOn ||
             component.type == kTimerOff) {
    const Pin* common = rolePin(PinRole::ContactCommon);
    const Pin* contact = rolePin(PinRole::ContactNo);
    if (component.type == kTimerOn || component.type == kTimerOff) {
      common = byName("15");
      contact = byName("18");
    }
    first = common;
    second = contact;
  } else if (component.type == kContactorAuxNc || component.type == kPushbuttonNc ||
             component.type == kEmergencyStop) {
    first = byName("1");
    second = byName("2");
    if (component.type == kContactorAuxNc) {
      first = byName("21");
      second = byName("22");
    } else if (component.type == kPushbuttonNc) {
      first = byName("21");
      second = byName("22");
    }
    if (first == nullptr || second == nullptr) {
      first = rolePin(PinRole::ContactCommon);
      second = rolePin(PinRole::ContactNc);
    }
  } else if (component.type == kThermalRelay) {
    // The two contact sets are independent. The caller handles both via passThroughPairs.
    first = nullptr;
    second = nullptr;
  } else if (component.type == kSelectorSwitch) {
    first = nullptr;
    second = nullptr;
  } else if (component.type == kTerminal) {
    first = nullptr;
    second = nullptr;
  }
  if (first == nullptr || second == nullptr) {
    return std::nullopt;
  }
  return std::make_pair(PinRef{component.id, first->id}, PinRef{component.id, second->id});
}

[[nodiscard]] std::vector<std::pair<PinRef, PinRef>> passThroughPairs(const Component& component) {
  using namespace component_type;
  std::vector<std::pair<PinRef, PinRef>> result;
  if (component.type == kThermalRelay) {
    const Pin* common_nc = component.findPin("95");
    const Pin* nc = component.findPin("96");
    const Pin* common_no = component.findPin("97");
    const Pin* no = component.findPin("98");
    if (common_nc != nullptr && nc != nullptr) {
      result.emplace_back(PinRef{component.id, common_nc->id}, PinRef{component.id, nc->id});
    }
    if (common_no != nullptr && no != nullptr) {
      result.emplace_back(PinRef{component.id, common_no->id}, PinRef{component.id, no->id});
    }
    return result;
  }
  if (component.type == kSelectorSwitch) {
    const Pin* common = component.findPin("1");
    if (common != nullptr) {
      for (const std::string_view name : {"2", "3", "4"}) {
        if (const Pin* position = component.findPin(name)) {
          result.emplace_back(PinRef{component.id, common->id}, PinRef{component.id, position->id});
        }
      }
    }
    return result;
  }
  if (const auto pair = passThroughPair(component)) {
    result.push_back(*pair);
  } else if (component.type == kSupplyAc3 || component.type == kSupplyAc1 ||
             component.type == kSupplyDc || component.type == kContactorCoil ||
             component.type == kPilotLamp || component.type == kMotor3ph ||
             component.type == kLabelBox) {
    // Supplies are sources, loads are not internally bridged for connectivity checks, and
    // label boxes have no electrical meaning.
  }
  return result;
}

[[nodiscard]] bool isCoilContact(const Component& component) {
  return isContactorContact(component.type);
}

[[nodiscard]] bool isContactorCoil(const Component& component) {
  return component.type == component_type::kContactorCoil;
}

struct ConnectivityGraph {
  std::map<PinRef, int> indices;
  DisjointSet sets;
  std::map<int, std::set<std::string>> sources;

  void ensure(const PinRef& pin) {
    if (!indices.contains(pin)) {
      indices.emplace(pin, sets.add());
    }
  }
  [[nodiscard]] int index(const PinRef& pin) {
    ensure(pin);
    return indices.at(pin);
  }
  void connect(const PinRef& left, const PinRef& right) { sets.unite(index(left), index(right)); }
  void addSource(const PinRef& pin, std::string group) {
    const int root = sets.find(index(pin));
    sources[root].insert(std::move(group));
  }
  [[nodiscard]] std::set<std::string> sourceGroups(const PinRef& pin) {
    const int root = sets.find(index(pin));
    const auto iterator = sources.find(root);
    return iterator == sources.end() ? std::set<std::string>{} : iterator->second;
  }
};

[[nodiscard]] ConnectivityGraph buildGraph(const Sheet& sheet) {
  ConnectivityGraph graph;
  for (const Component& component : sheet.components) {
    for (const Pin& pin : component.pins) {
      graph.ensure(PinRef{component.id, pin.id});
    }
  }
  for (const Wire& wire : sheet.wires) {
    graph.connect(wire.from.toPinRef(), wire.to.toPinRef());
  }
  for (const Component& component : sheet.components) {
    for (const auto& [left, right] : passThroughPairs(component)) {
      graph.connect(left, right);
    }
    for (const Pin& pin : component.pins) {
      if (pin.role == PinRole::SupplyIn || pin.role == PinRole::SupplyOut ||
          pin.role == PinRole::Neutral) {
        if (pin.role == PinRole::Earth) {
          continue;
        }
        graph.addSource(PinRef{component.id, pin.id}, sourceGroup(component, pin));
      }
    }
  }
  return graph;
}

[[nodiscard]] std::string pinLabel(const Component& component, const Pin& pin) {
  return component.tag + ":" + pin.name;
}

} // namespace

std::string_view toString(ProblemSeverity severity) {
  switch (severity) {
  case ProblemSeverity::Error:
    return "error";
  case ProblemSeverity::Warning:
    return "warning";
  case ProblemSeverity::Info:
    return "info";
  }
  return "warning";
}

std::string_view toString(ProblemCode code) {
  switch (code) {
  case ProblemCode::DuplicateTag:
    return "duplicate_tag";
  case ProblemCode::DuplicateId:
    return "duplicate_id";
  case ProblemCode::FloatingWire:
    return "floating_wire";
  case ProblemCode::InvalidWireEndpoint:
    return "invalid_wire_endpoint";
  case ProblemCode::SelfLoopNotAllowed:
    return "self_loop_not_allowed";
  case ProblemCode::RequiredPinUnconnected:
    return "required_pin_unconnected";
  case ProblemCode::UnmatchedContact:
    return "unmatched_contact";
  case ProblemCode::CoilWithoutContact:
    return "coil_without_contact";
  case ProblemCode::MissingSupplyPath:
    return "missing_supply_path";
  case ProblemCode::ComponentOverlap:
    return "component_overlap";
  case ProblemCode::InvalidComponent:
    return "invalid_component";
  case ProblemCode::InvalidPinConnection:
    return "invalid_pin_connection";
  }
  return "unknown";
}

bool ValidationReport::hasErrors() const {
  return std::any_of(problems.begin(), problems.end(), [](const Problem& problem) {
    return problem.severity == ProblemSeverity::Error;
  });
}

bool ValidationReport::hasWarnings() const {
  return std::any_of(problems.begin(), problems.end(), [](const Problem& problem) {
    return problem.severity == ProblemSeverity::Warning;
  });
}

std::size_t ValidationReport::errorCount() const {
  return static_cast<std::size_t>(
      std::count_if(problems.begin(), problems.end(), [](const Problem& problem) {
        return problem.severity == ProblemSeverity::Error;
      }));
}

std::size_t ValidationReport::warningCount() const {
  return static_cast<std::size_t>(
      std::count_if(problems.begin(), problems.end(), [](const Problem& problem) {
        return problem.severity == ProblemSeverity::Warning;
      }));
}

std::size_t ValidationReport::infoCount() const {
  return static_cast<std::size_t>(
      std::count_if(problems.begin(), problems.end(), [](const Problem& problem) {
        return problem.severity == ProblemSeverity::Info;
      }));
}

void ValidationReport::add(Problem problem) { problems.push_back(std::move(problem)); }

bool coilHasSupplyPath(const Diagram& diagram, const Component& coil) {
  const Sheet* sheet = diagram.findSheetForComponent(coil.id);
  if (sheet == nullptr) {
    return false;
  }
  ConnectivityGraph graph = buildGraph(*sheet);
  const Pin* a1 = coil.findPin("A1");
  const Pin* a2 = coil.findPin("A2");
  if (a1 == nullptr || a2 == nullptr) {
    return false;
  }
  const auto first_groups = graph.sourceGroups(PinRef{coil.id, a1->id});
  const auto second_groups = graph.sourceGroups(PinRef{coil.id, a2->id});
  if (first_groups.empty() || second_groups.empty()) {
    return false;
  }
  if (first_groups.size() == 1 && second_groups.size() == 1 &&
      *first_groups.begin() == *second_groups.begin()) {
    return false;
  }
  return true;
}

ValidationReport validate(const Diagram& diagram) {
  ValidationReport report;

  // Global tag uniqueness and id integrity.
  std::map<std::string, std::vector<ComponentId>> tags;
  for (const Sheet& sheet : diagram.sheets) {
    for (const Component& component : sheet.components) {
      tags[component.tag].push_back(component.id);
      if (component.tag.empty()) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::InvalidComponent,
                    "Component has an empty tag",
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
      if (component.type.empty()) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::InvalidComponent,
                    "Component has an empty type",
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
      if (component.pins.empty() && component.type != component_type::kLabelBox) {
        report.add({ProblemSeverity::Warning,
                    ProblemCode::InvalidComponent,
                    "Component has no electrical pins: " + component.tag,
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
    }
  }
  for (const auto& [tag, ids] : tags) {
    if (ids.size() < 2) {
      continue;
    }
    for (std::size_t index = 1; index < ids.size(); ++index) {
      const Sheet* sheet = diagram.findSheetForComponent(ids[index]);
      report.add({ProblemSeverity::Error,
                  ProblemCode::DuplicateTag,
                  "Duplicate tag: " + tag,
                  sheet == nullptr ? SheetId{} : sheet->id,
                  ids[index],
                  std::nullopt,
                  std::nullopt});
    }
  }

  for (const Sheet& sheet : diagram.sheets) {
    // Wires.
    std::set<WireId> wire_ids;
    for (const Wire& wire : sheet.wires) {
      if (!wire_ids.insert(wire.id).second) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::DuplicateId,
                    "Duplicate wire id",
                    sheet.id,
                    std::nullopt,
                    wire.id,
                    std::nullopt});
      }
      const Component* from_component = sheet.findComponent(wire.from.component);
      const Component* to_component = sheet.findComponent(wire.to.component);
      const Pin* from_pin = from_component == nullptr ? nullptr
                                                      : from_component->findPin(wire.from.pin);
      const Pin* to_pin = to_component == nullptr ? nullptr : to_component->findPin(wire.to.pin);
      if (from_component == nullptr || to_component == nullptr || from_pin == nullptr ||
          to_pin == nullptr) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::FloatingWire,
                    "Wire has a missing endpoint",
                    sheet.id,
                    std::nullopt,
                    wire.id,
                    std::nullopt});
        continue;
      }
      if (wire.from.pin == wire.to.pin) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::InvalidWireEndpoint,
                    "Wire connects a pin to itself",
                    sheet.id,
                    from_component->id,
                    wire.id,
                    PinRef{from_component->id, from_pin->id}});
      }
      if (from_component->id == to_component->id && !wire.allow_self_loop) {
        report.add({ProblemSeverity::Error,
                    ProblemCode::SelfLoopNotAllowed,
                    "Self-loop wire is not allowed",
                    sheet.id,
                    from_component->id,
                    wire.id,
                    std::nullopt});
      }
      for (const Pin* pin : {from_pin, to_pin}) {
        if (std::find(pin->wires.begin(), pin->wires.end(), wire.id) == pin->wires.end()) {
          report.add({ProblemSeverity::Error,
                      ProblemCode::InvalidPinConnection,
                      "Pin does not list its wire",
                      sheet.id,
                      from_component->id,
                      wire.id,
                      PinRef{from_component->id, pin->id}});
        }
      }
    }

    // Pin connection references and required pin rules.
    for (const Component& component : sheet.components) {
      for (const Pin& pin : component.pins) {
        for (const WireId wire_id : pin.wires) {
          const Wire* wire = sheet.findWire(wire_id);
          if (wire == nullptr) {
            report.add({ProblemSeverity::Error,
                        ProblemCode::InvalidPinConnection,
                        "Pin references a missing wire: " + pinLabel(component, pin),
                        sheet.id,
                        component.id,
                        wire_id,
                        PinRef{component.id, pin.id}});
            continue;
          }
          const bool references_pin =
              (wire->from.pin == pin.id && wire->from.component == component.id) ||
              (wire->to.pin == pin.id && wire->to.component == component.id);
          if (!references_pin) {
            report.add({ProblemSeverity::Error,
                        ProblemCode::InvalidPinConnection,
                        "Pin/wire back-reference mismatch: " + pinLabel(component, pin),
                        sheet.id,
                        component.id,
                        wire_id,
                        PinRef{component.id, pin.id}});
          }
        }
        if (pin.required && pin.wires.empty()) {
          report.add({ProblemSeverity::Warning,
                      ProblemCode::RequiredPinUnconnected,
                      "Required pin is not connected: " + pinLabel(component, pin),
                      sheet.id,
                      component.id,
                      std::nullopt,
                      PinRef{component.id, pin.id}});
        }
      }
    }

    // Contactor coil/contact pairing.
    std::set<std::string> coil_groups;
    std::set<std::string> contact_groups;
    for (const Component& component : sheet.components) {
      if (isContactorCoil(component)) {
        coil_groups.insert(component.labelGroup());
      }
      if (isCoilContact(component)) {
        contact_groups.insert(component.labelGroup());
      }
    }
    for (const Component& component : sheet.components) {
      if (isCoilContact(component) && !coil_groups.contains(component.labelGroup())) {
        report.add({ProblemSeverity::Warning,
                    ProblemCode::UnmatchedContact,
                    "Contact has no matching coil for label group " + component.labelGroup(),
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
      if (isContactorCoil(component) && !contact_groups.contains(component.labelGroup())) {
        report.add({ProblemSeverity::Info,
                    ProblemCode::CoilWithoutContact,
                    "Coil has no matching contacts: " + component.tag,
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
    }

    // Coil supply path.
    ConnectivityGraph graph = buildGraph(sheet);
    for (const Component& component : sheet.components) {
      if (!isCoil(component.type)) {
        continue;
      }
      const Pin* a1 = component.findPin("A1");
      const Pin* a2 = component.findPin("A2");
      if (a1 == nullptr || a2 == nullptr) {
        report.add({ProblemSeverity::Warning,
                    ProblemCode::MissingSupplyPath,
                    "Coil has no A1/A2 pin pair: " + component.tag,
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
        continue;
      }
      const auto first_groups = graph.sourceGroups(PinRef{component.id, a1->id});
      const auto second_groups = graph.sourceGroups(PinRef{component.id, a2->id});
      bool path_ok = !first_groups.empty() && !second_groups.empty();
      if (path_ok && first_groups.size() == 1 && second_groups.size() == 1 &&
          *first_groups.begin() == *second_groups.begin()) {
        path_ok = false;
      }
      if (!path_ok) {
        report.add({ProblemSeverity::Warning,
                    ProblemCode::MissingSupplyPath,
                    "Coil has no complete supply path: " + component.tag,
                    sheet.id,
                    component.id,
                    std::nullopt,
                    std::nullopt});
      }
    }

    // Component bounds that fully overlap are likely editing mistakes.
    for (std::size_t i = 0; i < sheet.components.size(); ++i) {
      for (std::size_t j = i + 1; j < sheet.components.size(); ++j) {
        const Component& left = sheet.components[i];
        const Component& right = sheet.components[j];
        const Rect a = left.boundingBox();
        const Rect b = right.boundingBox();
        if (!a.intersects(b)) {
          continue;
        }
        const double overlap_width = std::min(a.right(), b.right()) - std::max(a.left(), b.left());
        const double overlap_height = std::min(a.bottom(), b.bottom()) - std::max(a.top(), b.top());
        if (overlap_width > 2.0 && overlap_height > 2.0) {
          report.add({ProblemSeverity::Warning,
                      ProblemCode::ComponentOverlap,
                      "Components overlap: " + left.tag + " and " + right.tag,
                      sheet.id,
                      right.id,
                      std::nullopt,
                      std::nullopt});
        }
      }
    }
  }

  std::stable_sort(report.problems.begin(),
                   report.problems.end(),
                   [](const Problem& left, const Problem& right) {
                     if (left.sheet != right.sheet) {
                       return left.sheet < right.sheet;
                     }
                     if (left.severity != right.severity) {
                       return static_cast<int>(left.severity) < static_cast<int>(right.severity);
                     }
                     return toString(left.code) < toString(right.code);
                   });

  return report;
}

} // namespace fiovivo
