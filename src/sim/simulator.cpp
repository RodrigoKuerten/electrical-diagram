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

[[nodiscard]] const Pin* pinByRole(const Component& component, PinRole role) {
  for (const Pin& pin : component.pins) {
    if (pin.role == role) {
      return &pin;
    }
  }
  return nullptr;
}

[[nodiscard]] const Pin* pinByName(const Component& component, std::string_view name) {
  return component.findPin(name);
}

[[nodiscard]] bool isLamp(const Component& component) {
  return component.type == component_type::kPilotLamp;
}

[[nodiscard]] bool isMotor(const Component& component) {
  return component.type == component_type::kMotor3ph;
}

[[nodiscard]] bool isContactorContactType(std::string_view type) {
  return type == component_type::kContactorNo || type == component_type::kContactorAuxNo ||
         type == component_type::kContactorAuxNc;
}

[[nodiscard]] bool isNormallyOpenContact(const Component& component) {
  return component.type != component_type::kContactorAuxNc;
}

[[nodiscard]] std::optional<std::pair<const Pin*, const Pin*>>
contactorContactPins(const Component& component) {
  const Pin* common = pinByRole(component, PinRole::ContactCommon);
  const Pin* contact = pinByRole(component, PinRole::ContactNo);
  if (contact == nullptr) {
    contact = pinByRole(component, PinRole::ContactNc);
  }
  if (common == nullptr || contact == nullptr) {
    return std::nullopt;
  }
  return std::make_pair(common, contact);
}

[[nodiscard]] std::vector<PinRef> loadTerminals(const Component& component) {
  std::vector<PinRef> terminals;
  const auto add = [&component, &terminals](std::string_view name) {
    if (const Pin* pin = pinByName(component, name)) {
      terminals.emplace_back(component.id, pin->id);
    }
  };
  if (component.type == component_type::kMotor3ph) {
    add("U1");
    add("V1");
    add("W1");
  } else if (component.type == component_type::kPilotLamp ||
             component.type == component_type::kContactorCoil ||
             component.type == component_type::kTimerOn ||
             component.type == component_type::kTimerOff) {
    add("A1");
    add("A2");
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
      if (item.type == component_type::kSelectorSwitch) {
        const int positions =
            std::max(1, static_cast<int>(parameterInt(item.parameters, "positions", 2)));
        input.selector_position = std::clamp(
            static_cast<int>(parameterInt(item.parameters, "position", 1)), 1, positions);
      }
      if (item.type == component_type::kFuse) {
        input.tripped = parameterBool(item.parameters, "blown", false);
      } else if (item.type == component_type::kBreaker) {
        input.tripped = parameterBool(item.parameters, "tripped", false);
      } else if (item.type == component_type::kThermalRelay) {
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

  const BaseGraph base = buildBaseGraph();
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
  if (!isPushbutton(item->type) && item->type != component_type::kEmergencyStop) {
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
  if (item->type != component_type::kFuse && item->type != component_type::kBreaker &&
      item->type != component_type::kThermalRelay) {
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
  if (item->type != component_type::kSelectorSwitch) {
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
  const auto connectByName =
      [&connect](const Component& item, std::string_view first, std::string_view second) {
        const Pin* left = item.findPin(first);
        const Pin* right = item.findPin(second);
        if (left != nullptr && right != nullptr) {
          connect(PinRef{item.id, left->id}, PinRef{item.id, right->id});
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
      if (isContactorContactType(item.type)) {
        if (const auto pins = contactorContactPins(item)) {
          const std::optional<int> first = indexOf(PinRef{item.id, pins->first->id});
          const std::optional<int> second = indexOf(PinRef{item.id, pins->second->id});
          if (first.has_value() && second.has_value()) {
            base.contactor_edges.push_back(
                ContactorEdge{*first, *second, item.labelGroup(), isNormallyOpenContact(item)});
          }
        }
        continue;
      }
      if (item.type == component_type::kPushbuttonNo) {
        if (input.active) {
          connectByName(item, "13", "14");
        }
        continue;
      }
      if (item.type == component_type::kPushbuttonNc) {
        if (!input.active) {
          connectByName(item, "21", "22");
        }
        continue;
      }
      if (item.type == component_type::kEmergencyStop) {
        if (!input.active) {
          connectByName(item, "1", "2");
        }
        continue;
      }
      if (item.type == component_type::kFuse || item.type == component_type::kBreaker) {
        if (!input.tripped) {
          connectByName(item, "1", "2");
        }
        continue;
      }
      if (item.type == component_type::kThermalRelay) {
        if (!input.tripped) {
          connectByName(item, "95", "96");
        } else {
          connectByName(item, "97", "98");
        }
        continue;
      }
      if (item.type == component_type::kSelectorSwitch) {
        const std::string target = std::to_string(std::max(1, input.selector_position) + 1);
        connectByName(item, "1", target);
        continue;
      }
      if (item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) {
        if (state.timer_output) {
          connectByName(item, "15", "18");
        }
        continue;
      }
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
      if (!isContactorCoilComponent(item)) {
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
      if (item.type != component_type::kTimerOn && item.type != component_type::kTimerOff) {
        continue;
      }
      ComponentState& state = states_[item.id];
      const bool previous_output = state.timer_output;
      const std::int64_t delay = delayMs(item);
      const std::int64_t cycle = options_.cycle_ms;
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
      changed = changed || previous_output != state.timer_output;
    }
  }
  return changed;
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
      if (!isLamp(item) && !isMotor(item)) {
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
      } else if (item.type == component_type::kPushbuttonNo) {
        state.closed = input.active;
      } else if (item.type == component_type::kPushbuttonNc) {
        state.closed = !input.active;
      } else if (item.type == component_type::kEmergencyStop) {
        state.closed = !input.active;
      } else if (item.type == component_type::kFuse || item.type == component_type::kBreaker) {
        state.closed = !input.tripped;
      } else if (item.type == component_type::kThermalRelay) {
        state.closed = !input.tripped;
      } else if (item.type == component_type::kSelectorSwitch) {
        state.closed = true;
      } else if (item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) {
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
  if (item.type == component_type::kPushbuttonNo) {
    return input.active;
  }
  if (item.type == component_type::kPushbuttonNc) {
    return !input.active;
  }
  if (item.type == component_type::kEmergencyStop) {
    return !input.active;
  }
  if (item.type == component_type::kFuse || item.type == component_type::kBreaker) {
    return !input.tripped;
  }
  if (item.type == component_type::kThermalRelay) {
    return !input.tripped;
  }
  if (item.type == component_type::kSelectorSwitch) {
    return true;
  }
  if (item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) {
    return false;
  }
  return false;
}

const Component* Simulator::component(ComponentId id) const { return diagram_.findComponent(id); }

DigitalInputState Simulator::inputFor(ComponentId id) const {
  const auto iterator = inputs_.find(id);
  return iterator == inputs_.end() ? DigitalInputState{} : iterator->second;
}

bool Simulator::isContactorCoilComponent(const Component& item) const {
  return item.type == component_type::kContactorCoil;
}

std::int64_t Simulator::delayMs(const Component& item) const {
  std::int64_t delay = parameterInt(item.parameters, "delay_ms", 0);
  if (parameterString(item.parameters, "time_base", "ms") == "s") {
    delay *= 1000;
  }
  return std::max<std::int64_t>(0, delay);
}

} // namespace fiovivo::sim
