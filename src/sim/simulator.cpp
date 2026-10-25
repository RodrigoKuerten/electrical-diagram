#include "sim/simulator.hpp"

#include "core/types.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <string_view>
#include <utility>

namespace fiovivo::sim {
namespace {

constexpr std::string_view kStateCoil = "coil";
constexpr std::string_view kStateContact = "contact";
constexpr std::string_view kStateTimer = "timer";
constexpr std::string_view kStateLamp = "lamp";
constexpr std::string_view kStateMotor = "motor";
constexpr std::string_view kStateEmergencyStop = "emergency_stop";
constexpr std::string_view kStateInput = "input";
constexpr std::string_view kStateTripped = "tripped";
constexpr std::string_view kStatePosition = "position";

[[nodiscard]] const Pin* pinByName(const Component& component, std::string_view name) {
  return component.findPin(name);
}

[[nodiscard]] bool startsWithT(std::string_view value, std::string_view prefix) {
  return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] bool isLamp(const Component& component) {
  return component.type == component_type::kPilotLamp ||
         startsWithT(component.type, "pilot_lamp") || startsWithT(component.type, "lamp_") ||
         component.type == "led";
}

[[nodiscard]] bool isMotor(const Component& component) {
  return component.type == component_type::kMotor3ph || startsWithT(component.type, "motor_") ||
         component.type == "linear_actuator";
}

[[nodiscard]] bool isProtectionType(std::string_view type) {
  return type == component_type::kFuse || type == component_type::kBreaker ||
         startsWithT(type, "fuse") || startsWithT(type, "breaker") || startsWithT(type, "rcd_") ||
         type == "motor_breaker_3p";
}

[[nodiscard]] bool isThermalRelayType(std::string_view type) {
  return type == component_type::kThermalRelay;
}

[[nodiscard]] bool isSelectorType(std::string_view type) {
  return type == component_type::kSelectorSwitch || startsWithT(type, "selector_");
}

[[nodiscard]] bool isTimedContactType(std::string_view type) {
  return startsWithT(type, "contact_timed_");
}

[[nodiscard]] bool isThermalContactType(std::string_view type) {
  return startsWithT(type, "contact_thermal_");
}

[[nodiscard]] bool isLogicType(std::string_view type) { return startsWithT(type, "logic_"); }

[[nodiscard]] bool isGroupContactType(std::string_view type) {
  return type == component_type::kContactorNo || startsWithT(type, "contactor_aux_") ||
         startsWithT(type, "relay_contact_") || startsWithT(type, "contact_") ||
         startsWithT(type, "contactor_") || startsWithT(type, "relay_combo");
}

[[nodiscard]] bool isContactorContactType(std::string_view type) {
  return isGroupContactType(type) && !isThermalContactType(type) && !isTimedContactType(type);
}

struct ContactCandidate {
  const Pin* first = nullptr;
  const Pin* second = nullptr;
  bool normally_open = true;
  bool group_controlled = false;
};

[[nodiscard]] std::vector<ContactCandidate> contactCandidates(const Component& component) {
  std::vector<ContactCandidate> result;
  std::set<std::pair<PinId, PinId>> seen;
  const auto add =
      [&result,
       &seen](const Pin* first, const Pin* second, bool normally_open, bool group_controlled) {
        if (first == nullptr || second == nullptr || first->id == second->id) {
          return;
        }
        const auto key = first->id < second->id ? std::make_pair(first->id, second->id)
                                                : std::make_pair(second->id, first->id);
        if (!seen.insert(key).second) {
          return;
        }
        result.push_back(ContactCandidate{first, second, normally_open, group_controlled});
      };

  const bool group = isGroupContactType(component.type) && !isThermalContactType(component.type) &&
                     !isTimedContactType(component.type);
  if (isThermalRelayType(component.type)) {
    add(component.findPin("95"), component.findPin("96"), false, false);
    add(component.findPin("97"), component.findPin("98"), true, false);
    return result;
  }
  const bool two_terminal = isProtectionType(component.type);
  if (two_terminal) {
    // Main poles and neutral pole. Legacy devices use 1/2 for a single pole.
    static constexpr std::string_view tops[] = {"1", "3", "5", "7", "N"};
    static constexpr std::string_view bottoms[] = {"2", "4", "6", "8", "N2"};
    for (std::size_t i = 0; i < std::size(tops); ++i) {
      add(component.findPin(tops[i]), component.findPin(bottoms[i]), true, false);
    }
  }

  std::vector<const Pin*> commons;
  std::vector<const Pin*> normally_open;
  std::vector<const Pin*> normally_closed;
  for (const Pin& pin : component.pins) {
    if (pin.role == PinRole::ContactCommon) {
      commons.push_back(&pin);
    } else if (pin.role == PinRole::ContactNo) {
      normally_open.push_back(&pin);
    } else if (pin.role == PinRole::ContactNc) {
      normally_closed.push_back(&pin);
    }
  }
  for (std::size_t i = 0; i < commons.size(); ++i) {
    if (i < normally_open.size()) {
      add(commons[i], normally_open[i], true, group);
    }
    if (i < normally_closed.size()) {
      add(commons[i], normally_closed[i], false, group);
    }
  }
  // A changeover contact has one common and one NO + one NC pin.
  if (commons.size() == 1 && normally_open.size() == 1 && normally_closed.size() == 1) {
    add(commons.front(), normally_open.front(), true, group);
    add(commons.front(), normally_closed.front(), false, group);
  }
  // Contactor and relay auxiliaries use standard 13/14 and 21/22 numbering.
  add(component.findPin("13"), component.findPin("14"), true, group);
  add(component.findPin("21"), component.findPin("22"), false, group);
  return result;
}

[[nodiscard]] bool isNormallyOpenContact(const Component& component) {
  for (const ContactCandidate& candidate : contactCandidates(component)) {
    return candidate.normally_open;
  }
  return true;
}

[[nodiscard]] std::vector<PinRef> loadTerminals(const Component& component) {
  std::vector<PinRef> terminals;
  const auto add = [&component, &terminals](std::string_view name) {
    if (const Pin* pin = pinByName(component, name)) {
      terminals.emplace_back(component.id, pin->id);
    }
  };
  if (isMotor(component)) {
    if (component.findPin("U1") != nullptr) {
      add("U1");
      add("V1");
      add("W1");
    } else if (component.findPin("A1") != nullptr && component.findPin("A2") != nullptr) {
      add("A1");
      add("A2");
    } else if (component.findPin("+") != nullptr && component.findPin("-") != nullptr) {
      add("+");
      add("-");
    } else {
      for (const Pin& pin : component.pins) {
        if (pin.role != PinRole::Earth) {
          terminals.emplace_back(component.id, pin.id);
          if (terminals.size() == 2) {
            break;
          }
        }
      }
    }
  } else {
    add("A1");
    add("A2");
    if (terminals.empty()) {
      for (const Pin& pin : component.pins) {
        if (pin.role == PinRole::CoilInput || pin.role == PinRole::CoilOutput ||
            pin.role == PinRole::Terminal) {
          terminals.emplace_back(component.id, pin.id);
          if (terminals.size() == 2) {
            break;
          }
        }
      }
    }
  }
  return terminals;
}

} // namespace

int Simulator::Dsu::add() {
  const int index = static_cast<int>(parent.size());
  parent.push_back(index);
  rank.push_back(0);
  return index;
}

int Simulator::Dsu::find(int value) {
  if (parent[static_cast<std::size_t>(value)] != value) {
    parent[static_cast<std::size_t>(value)] = find(parent[static_cast<std::size_t>(value)]);
  }
  return parent[static_cast<std::size_t>(value)];
}

void Simulator::Dsu::unite(int left, int right) {
  int root_left = find(left);
  int root_right = find(right);
  if (root_left == root_right) {
    return;
  }
  if (rank[static_cast<std::size_t>(root_left)] < rank[static_cast<std::size_t>(root_right)]) {
    std::swap(root_left, root_right);
  }
  parent[static_cast<std::size_t>(root_right)] = root_left;
  if (rank[static_cast<std::size_t>(root_left)] == rank[static_cast<std::size_t>(root_right)]) {
    ++rank[static_cast<std::size_t>(root_left)];
  }
}

Simulator::Simulator(Diagram diagram, SimulationOptions options)
    : diagram_(std::move(diagram)), options_(options) {
  if (options_.cycle_ms <= 0) {
    options_.cycle_ms = 10;
  }
  if (options_.max_passes <= 0) {
    options_.max_passes = 16;
  }
  if (options_.max_log_entries == 0) {
    options_.max_log_entries = 1;
  }
  (void)reset();
}

Result<void> Simulator::reset() {
  states_.clear();
  inputs_.clear();
  live_wires_.clear();
  log_.clear();
  time_ms_ = 0;
  cycle_ = 0;
  fault_.reset();

  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      DigitalInputState input;
      if (isSelectorType(item.type)) {
        const int positions =
            std::max(1, static_cast<int>(parameterInt(item.parameters, "positions", 2)));
        input.selector_position = std::clamp(
            static_cast<int>(parameterInt(item.parameters, "position", 1)), 1, positions);
      }
      if (startsWithT(item.type, "switch_power")) {
        input.active = true;
      }
      if (isProtectionType(item.type)) {
        input.tripped = parameterBool(item.parameters, "blown", false) ||
                        parameterBool(item.parameters, "tripped", false);
      } else if (isThermalRelayType(item.type)) {
        input.tripped = parameterBool(item.parameters, "tripped", false);
      }
      inputs_[item.id] = input;

      ComponentState state;
      state.input_active = input.active;
      state.tripped = input.tripped;
      state.selector_position = input.selector_position;
      state.closed = initialClosed(item, input);
      if (item.type == component_type::kSelectorSwitch) {
        state.selector_position = input.selector_position;
      }
      states_[item.id] = state;
    }
  }

  BaseGraph base = buildBaseGraph();
  evaluateLogic(base);
  Dsu dsu = withContactorEdges(base, groupStates());
  recomputeLiveWires(dsu, base);
  emitFrame();
  return {};
}

Result<void> Simulator::step() {
  if (fault_.has_value()) {
    return *fault_;
  }
  if (options_.cycle_ms <= 0 || options_.max_passes <= 0) {
    return makeError(ErrorCode::SimulationError, "Invalid simulation options");
  }

  const std::map<ComponentId, ComponentState> previous_states = states_;
  const std::set<WireId> previous_live_wires = live_wires_;
  const std::size_t previous_log_size = log_.size();
  const std::int64_t previous_time = time_ms_;
  const std::uint64_t previous_cycle = cycle_;

  BaseGraph base = buildBaseGraph();
  evaluateLogic(base);
  Result<Dsu> solved = solveCoils(base);
  if (!solved) {
    states_ = previous_states;
    live_wires_ = previous_live_wires;
    time_ms_ = previous_time;
    cycle_ = previous_cycle;
    while (log_.size() > previous_log_size) {
      log_.pop_back();
    }
    fault_ = solved.error();
    if (observer_ != nullptr) {
      observer_->onFault(solved.error());
    }
    return solved.error();
  }

  Dsu final_dsu = std::move(solved).value();
  const bool timers_changed = advanceTimers();
  if (timers_changed) {
    base = buildBaseGraph();
    evaluateLogic(base);
    Result<Dsu> resolved = solveCoils(base);
    if (!resolved) {
      states_ = previous_states;
      live_wires_ = previous_live_wires;
      time_ms_ = previous_time;
      cycle_ = previous_cycle;
      while (log_.size() > previous_log_size) {
        log_.pop_back();
      }
      fault_ = resolved.error();
      if (observer_ != nullptr) {
        observer_->onFault(resolved.error());
      }
      return resolved.error();
    }
    final_dsu = std::move(resolved).value();
  }

  const std::map<std::string, bool> groups = groupStates();
  updateLoads(final_dsu, base);
  updateVisualState(final_dsu, base, groups);
  recomputeLiveWires(final_dsu, base);

  time_ms_ += options_.cycle_ms;
  ++cycle_;

  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      const auto previous = previous_states.find(item.id);
      const auto current = states_.find(item.id);
      if (previous != previous_states.end() && current != states_.end()) {
        logTransition(previous->second, current->second, item);
      }
    }
  }

  emitFrame();
  return {};
}

Result<void> Simulator::setInput(ComponentId component_id, bool active) {
  const Component* item = component(component_id);
  if (item == nullptr) {
    return makeError(ErrorCode::NotFound, "Component does not exist");
  }
  if (!isPushbutton(item->type) && item->type != component_type::kEmergencyStop &&
      !startsWithT(item->type, "switch_") && !startsWithT(item->type, "limit_") &&
      !startsWithT(item->type, "sensor_") && !startsWithT(item->type, "pressure_") &&
      !startsWithT(item->type, "vacuum_") && !startsWithT(item->type, "thermostat_") &&
      !startsWithT(item->type, "float_") && !startsWithT(item->type, "reed_")) {
    return makeError(ErrorCode::Unsupported, "Component is not a digital input", item->tag);
  }
  DigitalInputState& input = inputs_[component_id];
  if (input.active == active) {
    return {};
  }
  input.active = active;
  if (auto iterator = states_.find(component_id); iterator != states_.end()) {
    iterator->second.input_active = active;
    iterator->second.closed = initialClosed(*item, input);
  }
  logInput(component_id, std::string(kStateInput), active ? "on" : "off");
  emitFrame();
  return {};
}

Result<void> Simulator::setTripped(ComponentId component_id, bool tripped) {
  const Component* item = component(component_id);
  if (item == nullptr) {
    return makeError(ErrorCode::NotFound, "Component does not exist");
  }
  if (!isProtectionType(item->type) && !isThermalRelayType(item->type)) {
    return makeError(ErrorCode::Unsupported, "Component is not a protection device", item->tag);
  }
  DigitalInputState& input = inputs_[component_id];
  if (input.tripped == tripped) {
    return {};
  }
  input.tripped = tripped;
  if (auto iterator = states_.find(component_id); iterator != states_.end()) {
    iterator->second.tripped = tripped;
    iterator->second.closed = initialClosed(*item, input);
  }
  logInput(component_id, std::string(kStateTripped), tripped ? "on" : "off");
  emitFrame();
  return {};
}

Result<void> Simulator::setSelectorPosition(ComponentId component_id, int position) {
  const Component* item = component(component_id);
  if (item == nullptr) {
    return makeError(ErrorCode::NotFound, "Component does not exist");
  }
  if (!isSelectorType(item->type)) {
    return makeError(ErrorCode::Unsupported, "Component is not a selector switch", item->tag);
  }
  const int positions =
      std::max(1, static_cast<int>(parameterInt(item->parameters, "positions", 2)));
  if (position < 1 || position > positions) {
    return makeError(ErrorCode::InvalidArgument, "Selector position is out of range", item->tag);
  }
  DigitalInputState& input = inputs_[component_id];
  if (input.selector_position == position) {
    return {};
  }
  input.selector_position = position;
  if (auto iterator = states_.find(component_id); iterator != states_.end()) {
    iterator->second.selector_position = position;
  }
  logInput(component_id, std::string(kStatePosition), std::to_string(position));
  emitFrame();
  return {};
}

ComponentState Simulator::state(ComponentId component_id) const {
  const auto iterator = states_.find(component_id);
  return iterator == states_.end() ? ComponentState{} : iterator->second;
}

SimulationSnapshot Simulator::snapshot() const {
  SimulationSnapshot snapshot;
  snapshot.time_ms = time_ms_;
  snapshot.cycle = cycle_;
  snapshot.inputs = inputs_;
  snapshot.states = states_;
  return snapshot;
}

Result<void> Simulator::restore(const SimulationSnapshot& snapshot) {
  if (snapshot.time_ms < 0) {
    return makeError(ErrorCode::InvalidArgument, "Snapshot time must not be negative");
  }
  for (const auto& [id, state] : snapshot.states) {
    (void)state;
    if (component(id) == nullptr) {
      return makeError(ErrorCode::InvalidReference, "Snapshot references unknown component");
    }
  }
  for (const auto& [id, input] : snapshot.inputs) {
    (void)input;
    if (component(id) == nullptr) {
      return makeError(ErrorCode::InvalidReference, "Snapshot references unknown input");
    }
  }
  std::map<ComponentId, ComponentState> restored_states = snapshot.states;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (!restored_states.contains(item.id)) {
        return makeError(
            ErrorCode::InvalidReference, "Snapshot is missing a component state", item.tag);
      }
    }
  }
  states_ = std::move(restored_states);
  inputs_ = snapshot.inputs;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (!inputs_.contains(item.id)) {
        DigitalInputState input;
        if (item.type == component_type::kSelectorSwitch) {
          input.selector_position = states_.at(item.id).selector_position;
        }
        input.tripped = states_.at(item.id).tripped;
        input.active = states_.at(item.id).input_active;
        inputs_[item.id] = input;
      }
    }
  }
  time_ms_ = snapshot.time_ms;
  cycle_ = snapshot.cycle;
  fault_.reset();
  log_.clear();

  const BaseGraph base = buildBaseGraph();
  Dsu dsu = withContactorEdges(base, groupStates());
  updateVisualState(dsu, base, groupStates());
  recomputeLiveWires(dsu, base);
  emitFrame();
  return {};
}

Simulator::BaseGraph Simulator::buildBaseGraph() const {
  BaseGraph base;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      for (const Pin& pin : item.pins) {
        base.pin_indices.emplace(PinRef{item.id, pin.id}, base.dsu.add());
      }
    }
  }

  const auto indexOf = [&base](PinRef ref) -> std::optional<int> {
    const auto iterator = base.pin_indices.find(ref);
    if (iterator == base.pin_indices.end()) {
      return std::nullopt;
    }
    return iterator->second;
  };
  const auto connect = [&base, &indexOf](PinRef left, PinRef right) {
    const std::optional<int> left_index = indexOf(left);
    const std::optional<int> right_index = indexOf(right);
    if (left_index.has_value() && right_index.has_value()) {
      base.dsu.unite(*left_index, *right_index);
    }
  };

  for (const Sheet& sheet : diagram_.sheets) {
    for (const Wire& wire : sheet.wires) {
      connect(wire.from.toPinRef(), wire.to.toPinRef());
    }
  }

  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      const DigitalInputState input = inputFor(item.id);
      const ComponentState state = states_.contains(item.id) ? states_.at(item.id)
                                                             : ComponentState{};
      if (isSupply(item.type)) {
        for (const Pin& pin : item.pins) {
          if (pin.role == PinRole::SupplyIn || pin.role == PinRole::SupplyOut ||
              pin.role == PinRole::Neutral) {
            const std::optional<int> pin_index = indexOf(PinRef{item.id, pin.id});
            if (pin_index.has_value()) {
              base.sources.push_back(SourcePin{*pin_index, item.type + ":" + pin.name});
            }
          }
        }
        continue;
      }
      if (isSelectorType(item.type)) {
        const Pin* common = item.findPin("1");
        const std::string target = std::to_string(std::clamp(input.selector_position, 1, 3) + 1);
        if (const Pin* selected = item.findPin(target); common != nullptr && selected != nullptr) {
          connect(PinRef{item.id, common->id}, PinRef{item.id, selected->id});
        }
        continue;
      }
      if (isThermalContactType(item.type)) {
        bool tripped = false;
        for (const Sheet& other_sheet : diagram_.sheets) {
          for (const Component& other : other_sheet.components) {
            if (isThermalRelayType(other.type) && other.labelGroup() == item.labelGroup()) {
              const auto state_it = states_.find(other.id);
              tripped = tripped || (state_it != states_.end() && state_it->second.tripped);
            }
          }
        }
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          const bool closed = candidate.normally_open ? tripped : !tripped;
          if (closed) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      if (isTimedContactType(item.type)) {
        bool output = false;
        for (const Sheet& other_sheet : diagram_.sheets) {
          for (const Component& other : other_sheet.components) {
            if (isTimer(other.type) && other.labelGroup() == item.labelGroup()) {
              const auto state_it = states_.find(other.id);
              output = output || (state_it != states_.end() && state_it->second.timer_output);
            }
          }
        }
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          const bool closed = candidate.normally_open ? output : !output;
          if (closed) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      if (isProtectionType(item.type)) {
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          if (!input.tripped) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      if (isThermalRelayType(item.type)) {
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          const bool closed = candidate.normally_open ? input.tripped : !input.tripped;
          if (closed) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      if (item.type == component_type::kEmergencyStop || isPushbutton(item.type) ||
          startsWithT(item.type, "switch_") || startsWithT(item.type, "limit_") ||
          startsWithT(item.type, "sensor_") || startsWithT(item.type, "pressure_") ||
          startsWithT(item.type, "vacuum_") || startsWithT(item.type, "thermostat_") ||
          startsWithT(item.type, "float_") || startsWithT(item.type, "reed_")) {
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          const bool closed = candidate.normally_open ? input.active : !input.active;
          if (closed) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      if (isTimer(item.type)) {
        for (const ContactCandidate& candidate : contactCandidates(item)) {
          const bool closed = candidate.normally_open ? state.timer_output : !state.timer_output;
          if (closed) {
            connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
          }
        }
        continue;
      }
      for (const ContactCandidate& candidate : contactCandidates(item)) {
        if (candidate.group_controlled) {
          const std::optional<int> first = indexOf(PinRef{item.id, candidate.first->id});
          const std::optional<int> second = indexOf(PinRef{item.id, candidate.second->id});
          if (first.has_value() && second.has_value()) {
            base.contactor_edges.push_back(
                ContactorEdge{*first, *second, item.labelGroup(), candidate.normally_open});
          }
        } else {
          connect(PinRef{item.id, candidate.first->id}, PinRef{item.id, candidate.second->id});
        }
      }
    }
  }
  return base;
}

Simulator::Dsu
Simulator::withContactorEdges(const BaseGraph& base,
                              const std::map<std::string, bool>& group_states) const {
  Dsu dsu = base.dsu;
  for (const ContactorEdge& edge : base.contactor_edges) {
    const auto iterator = group_states.find(edge.group);
    const bool energized = iterator != group_states.end() && iterator->second;
    const bool closed = edge.normally_open ? energized : !energized;
    if (closed) {
      dsu.unite(edge.first, edge.second);
    }
  }
  return dsu;
}

std::set<std::string> Simulator::sourceGroups(Dsu& dsu, const BaseGraph& base, PinRef pin) const {
  const auto iterator = base.pin_indices.find(pin);
  if (iterator == base.pin_indices.end()) {
    return {};
  }
  const int root = dsu.find(iterator->second);
  std::set<std::string> groups;
  for (const SourcePin& source : base.sources) {
    if (dsu.find(source.index) == root) {
      groups.insert(source.group);
    }
  }
  return groups;
}

bool Simulator::hasDistinctSupply(Dsu& dsu,
                                  const BaseGraph& base,
                                  const std::vector<PinRef>& terminals) const {
  if (terminals.size() < 2) {
    return false;
  }
  std::vector<std::set<std::string>> groups;
  groups.reserve(terminals.size());
  for (const PinRef& terminal : terminals) {
    std::set<std::string> terminal_groups = sourceGroups(dsu, base, terminal);
    if (terminal_groups.empty()) {
      return false;
    }
    groups.push_back(std::move(terminal_groups));
  }
  for (std::size_t first = 0; first < groups.size(); ++first) {
    for (std::size_t second = first + 1; second < groups.size(); ++second) {
      for (const std::string& left : groups[first]) {
        for (const std::string& right : groups[second]) {
          if (left != right) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool Simulator::loadEnergized(Dsu& dsu, const BaseGraph& base, const Component& item) const {
  const std::vector<PinRef> terminals = loadTerminals(item);
  if (item.type == component_type::kMotor3ph) {
    const int min_phases =
        std::clamp(static_cast<int>(parameterInt(item.parameters, "min_live_phases", 2)), 1, 3);
    std::set<std::string> all_groups;
    int fed_terminals = 0;
    for (const PinRef& terminal : terminals) {
      std::set<std::string> groups = sourceGroups(dsu, base, terminal);
      if (!groups.empty()) {
        ++fed_terminals;
        all_groups.insert(groups.begin(), groups.end());
      }
    }
    return fed_terminals >= min_phases && all_groups.size() >= 2;
  }
  return hasDistinctSupply(dsu, base, terminals);
}

std::map<std::string, bool> Simulator::groupStates() const {
  std::map<std::string, bool> groups;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (!isCoil(item.type) || item.labelGroup().empty()) {
        continue;
      }
      const auto iterator = states_.find(item.id);
      const bool energized = iterator != states_.end() && iterator->second.energized;
      groups[item.labelGroup()] = groups[item.labelGroup()] || energized;
    }
  }
  return groups;
}

bool Simulator::advanceTimers() {
  bool changed = false;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (!isTimer(item.type)) {
        continue;
      }
      ComponentState& state = states_[item.id];
      const bool previous_output = state.timer_output;
      const std::int64_t cycle = options_.cycle_ms;
      if (item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) {
        const std::int64_t delay = delayMs(item);
        if (item.type == component_type::kTimerOn) {
          if (state.energized) {
            state.timer_elapsed_ms = std::min(delay, state.timer_elapsed_ms + cycle);
            state.timer_output = state.timer_elapsed_ms >= delay;
          } else {
            state.timer_elapsed_ms = 0;
            state.timer_output = false;
          }
        } else {
          if (state.energized) {
            state.timer_elapsed_ms = 0;
            state.timer_output = true;
          } else if (state.timer_output) {
            state.timer_elapsed_ms = std::min(delay, state.timer_elapsed_ms + cycle);
            if (state.timer_elapsed_ms >= delay) {
              state.timer_output = false;
            }
          } else {
            state.timer_elapsed_ms = 0;
          }
        }
      } else if (item.type == "timer_on_off") {
        const std::int64_t delay_on =
            std::max<std::int64_t>(0, parameterInt(item.parameters, "delay_on_ms", delayMs(item)));
        const std::int64_t delay_off =
            std::max<std::int64_t>(0, parameterInt(item.parameters, "delay_off_ms", delayMs(item)));
        if (state.energized) {
          state.timer_elapsed_ms = std::min(delay_on, state.timer_elapsed_ms + cycle);
          state.timer_output = state.timer_elapsed_ms >= delay_on;
        } else if (state.timer_output) {
          state.timer_elapsed_ms = std::min(delay_off, state.timer_elapsed_ms + cycle);
          if (state.timer_elapsed_ms >= delay_off) {
            state.timer_output = false;
          }
        } else {
          state.timer_elapsed_ms = 0;
        }
      } else if (item.type == "flasher") {
        if (state.energized) {
          const std::int64_t period =
              std::max<std::int64_t>(1, parameterInt(item.parameters, "period_ms", 1000));
          state.timer_elapsed_ms += cycle;
          if (state.timer_elapsed_ms >= period / 2) {
            state.timer_elapsed_ms = 0;
            state.timer_output = !state.timer_output;
          }
        } else {
          state.timer_elapsed_ms = 0;
          state.timer_output = false;
        }
      } else if (item.type == "timer_pulse") {
        const std::int64_t delay = delayMs(item);
        if (state.energized && !state.last_energized) {
          state.timer_elapsed_ms = 0;
          state.timer_output = true;
        } else if (state.energized && state.timer_output) {
          state.timer_elapsed_ms = std::min(delay, state.timer_elapsed_ms + cycle);
          if (state.timer_elapsed_ms >= delay) {
            state.timer_output = false;
          }
        } else if (!state.energized) {
          state.timer_elapsed_ms = 0;
          state.timer_output = false;
        }
      }
      state.last_energized = state.energized;
      changed = changed || previous_output != state.timer_output;
    }
  }
  return changed;
}

void Simulator::evaluateLogic(BaseGraph& base) {
  std::vector<const Component*> gates;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (isLogicType(item.type)) {
        gates.push_back(&item);
      }
    }
  }
  if (gates.empty()) {
    return;
  }
  std::stable_sort(gates.begin(), gates.end(), [](const Component* left, const Component* right) {
    if (left->tag != right->tag) {
      return left->tag < right->tag;
    }
    return left->id < right->id;
  });
  const std::vector<SourcePin> supply_sources = base.sources;
  for (int pass = 0; pass < 6; ++pass) {
    base.sources = supply_sources;
    Dsu dsu = withContactorEdges(base, groupStates());
    bool changed = false;
    for (const Component* gate : gates) {
      ComponentState& state = states_[gate->id];
      const auto active = [&](std::string_view name) {
        const Pin* pin = gate->findPin(name);
        if (pin == nullptr) {
          return false;
        }
        return !sourceGroups(dsu, base, PinRef{gate->id, pin->id}).empty();
      };
      const bool input = active("IN") || active("IN1");
      const bool input2 = active("IN2");
      bool output = state.logic_output;
      if (gate->type == "logic_and" || gate->type == "logic_nand") {
        output = input && input2;
        if (gate->type == "logic_nand") {
          output = !output;
        }
      } else if (gate->type == "logic_or" || gate->type == "logic_nor") {
        output = input || input2;
        if (gate->type == "logic_nor") {
          output = !output;
        }
      } else if (gate->type == "logic_not") {
        output = !input;
      } else if (gate->type == "logic_xor") {
        output = input != input2;
      } else if (gate->type == "logic_sr") {
        if (active("S")) {
          output = true;
        } else if (active("R")) {
          output = false;
        }
      } else if (gate->type == "logic_edge_up") {
        output = input && !state.last_input;
      } else if (gate->type == "logic_edge_down") {
        output = !input && state.last_input;
      } else if (gate->type == "logic_timer_on") {
        const std::int64_t delay = delayMs(*gate);
        if (input) {
          state.timer_elapsed_ms = std::min(delay, state.timer_elapsed_ms + options_.cycle_ms);
          output = state.timer_elapsed_ms >= delay;
        } else {
          state.timer_elapsed_ms = 0;
          output = false;
        }
      } else if (gate->type == "logic_counter") {
        if (input && !state.last_input) {
          ++state.counter;
        }
        const int preset = static_cast<int>(parameterInt(gate->parameters, "preset", 10));
        output = state.counter >= preset;
      }
      state.last_input = input;
      if (output != state.logic_output) {
        state.logic_output = output;
        changed = true;
      }
      if (output) {
        const Pin* out = gate->findPin("OUT");
        if (out != nullptr) {
          const auto index = base.pin_indices.find(PinRef{gate->id, out->id});
          if (index != base.pin_indices.end()) {
            base.sources.push_back(
                SourcePin{index->second, "logic:" + std::to_string(gate->id.value())});
          }
        }
      }
    }
    if (!changed) {
      break;
    }
  }
}

Result<Simulator::Dsu> Simulator::solveCoils(const BaseGraph& base) {
  std::vector<const Component*> coils;
  coils.reserve(diagram_.componentCount());
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (isCoil(item.type)) {
        coils.push_back(&item);
      }
    }
  }
  std::stable_sort(coils.begin(), coils.end(), [](const Component* left, const Component* right) {
    if (left->tag != right->tag) {
      return left->tag < right->tag;
    }
    return left->id < right->id;
  });

  for (int pass = 0; pass < options_.max_passes; ++pass) {
    bool changed = false;
    std::map<std::string, bool> groups = groupStates();
    if (base.contactor_edges.empty()) {
      // Without contactor contacts the graph is identical for every coil. Build it once.
      Dsu trial = base.dsu;
      for (const Component* coil : coils) {
        const bool energized = loadEnergized(trial, base, *coil);
        ComponentState& state = states_[coil->id];
        if (state.energized != energized) {
          state.energized = energized;
          changed = true;
        }
      }
      if (!changed) {
        return trial;
      }
      continue;
    }

    for (const Component* coil : coils) {
      Dsu trial = withContactorEdges(base, groups);
      const bool energized = loadEnergized(trial, base, *coil);
      ComponentState& state = states_[coil->id];
      if (state.energized != energized) {
        state.energized = energized;
        changed = true;
        if (isContactorCoilComponent(*coil)) {
          // Keep the shared group snapshot in sync for the next coil in this pass.
          bool any_on = false;
          const std::string group = coil->labelGroup();
          for (const Sheet& sheet : diagram_.sheets) {
            for (const Component& other : sheet.components) {
              if (isContactorCoilComponent(other) && other.labelGroup() == group) {
                any_on = any_on || states_[other.id].energized;
              }
            }
          }
          groups[group] = any_on;
        }
      }
    }
    if (!changed) {
      return withContactorEdges(base, groups);
    }
  }

  return makeError(ErrorCode::Oscillation,
                   "Simulation did not converge; possible electrical oscillation",
                   std::to_string(options_.max_passes) + " passes");
}

void Simulator::updateLoads(Dsu& final_dsu, const BaseGraph& base) {
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      if (!isLoad(item.type) || isCoil(item.type)) {
        continue;
      }
      states_[item.id].energized = loadEnergized(final_dsu, base, item);
    }
  }
}

void Simulator::updateVisualState(Dsu& final_dsu,
                                  const BaseGraph& base,
                                  const std::map<std::string, bool>& groups) {
  (void)final_dsu;
  (void)base;
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Component& item : sheet.components) {
      ComponentState& state = states_[item.id];
      const DigitalInputState input = inputFor(item.id);
      state.input_active = input.active;
      state.tripped = input.tripped;
      state.selector_position = input.selector_position;
      if (isContactorContactType(item.type)) {
        const auto iterator = groups.find(item.labelGroup());
        const bool energized = iterator != groups.end() && iterator->second;
        state.closed = isNormallyOpenContact(item) ? energized : !energized;
      } else if (isThermalContactType(item.type)) {
        bool tripped = false;
        for (const Sheet& other_sheet : diagram_.sheets) {
          for (const Component& other : other_sheet.components) {
            if (isThermalRelayType(other.type) && other.labelGroup() == item.labelGroup()) {
              const auto state_it = states_.find(other.id);
              tripped = tripped || (state_it != states_.end() && state_it->second.tripped);
            }
          }
        }
        state.closed = isNormallyOpenContact(item) ? tripped : !tripped;
      } else if (isTimedContactType(item.type)) {
        bool output = false;
        for (const Sheet& other_sheet : diagram_.sheets) {
          for (const Component& other : other_sheet.components) {
            if (isTimer(other.type) && other.labelGroup() == item.labelGroup()) {
              const auto state_it = states_.find(other.id);
              output = output || (state_it != states_.end() && state_it->second.timer_output);
            }
          }
        }
        state.closed = isNormallyOpenContact(item) ? output : !output;
      } else if (isPushbutton(item.type) || item.type == component_type::kEmergencyStop ||
                 startsWithT(item.type, "switch_") || startsWithT(item.type, "limit_") ||
                 startsWithT(item.type, "sensor_") || startsWithT(item.type, "pressure_") ||
                 startsWithT(item.type, "vacuum_") || startsWithT(item.type, "thermostat_") ||
                 startsWithT(item.type, "float_") || startsWithT(item.type, "reed_")) {
        state.closed = isNormallyOpenContact(item) ? input.active : !input.active;
      } else if (isProtectionType(item.type) || isThermalRelayType(item.type)) {
        state.closed = !input.tripped;
      } else if (isSelectorType(item.type)) {
        state.closed = true;
      } else if (isTimer(item.type)) {
        state.closed = state.timer_output;
      }
    }
  }
}

void Simulator::recomputeLiveWires(Dsu& final_dsu, const BaseGraph& base) {
  std::set<int> live_roots;
  for (const SourcePin& source : base.sources) {
    live_roots.insert(final_dsu.find(source.index));
  }
  live_wires_.clear();
  for (const Sheet& sheet : diagram_.sheets) {
    for (const Wire& wire : sheet.wires) {
      const auto first = base.pin_indices.find(wire.from.toPinRef());
      const auto second = base.pin_indices.find(wire.to.toPinRef());
      if (first == base.pin_indices.end() || second == base.pin_indices.end()) {
        continue;
      }
      if (live_roots.contains(final_dsu.find(first->second)) ||
          live_roots.contains(final_dsu.find(second->second))) {
        live_wires_.insert(wire.id);
      }
    }
  }
}

void Simulator::logTransition(const ComponentState& previous,
                              const ComponentState& current,
                              const Component& item) {
  if (isCoil(item.type) && previous.energized != current.energized) {
    appendLog(
        {time_ms_, item.id, item.tag, std::string(kStateCoil), current.energized ? "on" : "off"});
  }
  if (isContactorContactType(item.type) && previous.closed != current.closed) {
    appendLog({time_ms_,
               item.id,
               item.tag,
               std::string(kStateContact),
               current.closed ? "closed" : "open"});
  }
  if ((item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) &&
      previous.timer_output != current.timer_output) {
    appendLog({time_ms_,
               item.id,
               item.tag,
               std::string(kStateTimer),
               current.timer_output ? "active" : "rest"});
  }
  if (isLamp(item) && previous.energized != current.energized) {
    appendLog(
        {time_ms_, item.id, item.tag, std::string(kStateLamp), current.energized ? "on" : "off"});
  }
  if (isMotor(item) && previous.energized != current.energized) {
    appendLog({time_ms_,
               item.id,
               item.tag,
               std::string(kStateMotor),
               current.energized ? "running" : "stopped"});
  }
  if (item.type == component_type::kEmergencyStop &&
      previous.input_active != current.input_active) {
    appendLog({time_ms_,
               item.id,
               item.tag,
               std::string(kStateEmergencyStop),
               current.input_active ? "triggered" : "reset"});
  }
}

void Simulator::logInput(ComponentId component_id, std::string state, std::string value) {
  const Component* item = component(component_id);
  if (item == nullptr) {
    return;
  }
  appendLog({time_ms_, component_id, item->tag, std::move(state), std::move(value)});
}

void Simulator::appendLog(LogEntry entry) {
  if (log_.size() >= options_.max_log_entries) {
    log_.erase(log_.begin());
  }
  if (observer_ != nullptr) {
    observer_->onLog(entry);
  }
  log_.push_back(std::move(entry));
}

void Simulator::emitFrame() {
  if (observer_ == nullptr) {
    return;
  }
  SimulationFrame frame;
  frame.time_ms = time_ms_;
  frame.cycle = cycle_;
  frame.states = states_;
  frame.live_wires = live_wires_;
  observer_->onFrame(frame);
}

bool Simulator::initialClosed(const Component& item, const DigitalInputState& input) const {
  if (isContactorContactType(item.type)) {
    return !isNormallyOpenContact(item);
  }
  if (isSelectorType(item.type)) {
    return true;
  }
  if (isTimer(item.type)) {
    return false;
  }
  if (isProtectionType(item.type) || isThermalRelayType(item.type)) {
    return !input.tripped;
  }
  if (isThermalContactType(item.type) || isTimedContactType(item.type)) {
    return !isNormallyOpenContact(item);
  }
  if (isPushbutton(item.type) || item.type == component_type::kEmergencyStop ||
      startsWithT(item.type, "switch_") || startsWithT(item.type, "limit_") ||
      startsWithT(item.type, "sensor_") || startsWithT(item.type, "pressure_") ||
      startsWithT(item.type, "vacuum_") || startsWithT(item.type, "thermostat_") ||
      startsWithT(item.type, "float_") || startsWithT(item.type, "reed_")) {
    return isNormallyOpenContact(item) ? input.active : !input.active;
  }
  return false;
}

const Component* Simulator::component(ComponentId id) const { return diagram_.findComponent(id); }

DigitalInputState Simulator::inputFor(ComponentId id) const {
  const auto iterator = inputs_.find(id);
  return iterator == inputs_.end() ? DigitalInputState{} : iterator->second;
}

bool Simulator::isContactorCoilComponent(const Component& item) const { return isCoil(item.type); }

std::int64_t Simulator::delayMs(const Component& item) const {
  std::int64_t delay = parameterInt(item.parameters, "delay_ms", 0);
  if (delay == 0) {
    delay = parameterInt(item.parameters, "delay_on_ms", 0);
  }
  if (delay == 0) {
    delay = parameterInt(item.parameters, "period_ms", 0);
  }
  if (parameterString(item.parameters, "time_base", "ms") == "s") {
    delay *= 1000;
  }
  return std::max<std::int64_t>(0, delay);
}

} // namespace fiovivo::sim
