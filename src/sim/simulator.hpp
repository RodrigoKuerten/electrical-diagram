#pragma once

#include "core/ids.hpp"
#include "core/model.hpp"
#include "core/result.hpp"
#include "sim/simulation_types.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fiovivo::sim {

// Deterministic scan-cycle simulator. The simulator owns a copy of the diagram, so the UI can
// keep editing or reloading without invalidating a running simulation.
class Simulator {
public:
  explicit Simulator(Diagram diagram, SimulationOptions options = {});

  [[nodiscard]] const Diagram& diagram() const { return diagram_; }
  [[nodiscard]] const SimulationOptions& options() const { return options_; }

  Result<void> reset();
  Result<void> step();

  Result<void> setInput(ComponentId component_id, bool active);
  Result<void> setTripped(ComponentId component_id, bool tripped);
  Result<void> setSelectorPosition(ComponentId component_id, int position);

  [[nodiscard]] const std::map<ComponentId, ComponentState>& states() const { return states_; }
  [[nodiscard]] ComponentState state(ComponentId component_id) const;
  [[nodiscard]] const std::map<ComponentId, DigitalInputState>& inputs() const { return inputs_; }
  [[nodiscard]] const std::set<WireId>& liveWires() const { return live_wires_; }
  [[nodiscard]] const std::vector<LogEntry>& log() const { return log_; }
  [[nodiscard]] std::int64_t simulatedTimeMs() const { return time_ms_; }
  [[nodiscard]] std::uint64_t cycleCount() const { return cycle_; }
  [[nodiscard]] const std::optional<Error>& fault() const { return fault_; }
  [[nodiscard]] bool hasFault() const { return fault_.has_value(); }

  void setObserver(SimulationObserver* observer) { observer_ = observer; }

  [[nodiscard]] SimulationSnapshot snapshot() const;
  Result<void> restore(const SimulationSnapshot& snapshot);

private:
  struct Dsu {
    std::vector<int> parent;
    std::vector<int> rank;

    int add();
    int find(int value);
    void unite(int left, int right);
  };

  struct SourcePin {
    int index = 0;
    std::string group;
  };

  struct ContactorEdge {
    int first = 0;
    int second = 0;
    std::string group;
    bool normally_open = true;
  };

  struct BaseGraph {
    Dsu dsu;
    std::map<PinRef, int> pin_indices;
    std::vector<SourcePin> sources;
    std::vector<ContactorEdge> contactor_edges;
  };

  [[nodiscard]] BaseGraph buildBaseGraph() const;
  [[nodiscard]] Dsu withContactorEdges(const BaseGraph& base,
                                       const std::map<std::string, bool>& group_states) const;
  [[nodiscard]] std::set<std::string>
  sourceGroups(Dsu& dsu, const BaseGraph& base, PinRef pin) const;
  [[nodiscard]] bool
  hasDistinctSupply(Dsu& dsu, const BaseGraph& base, const std::vector<PinRef>& terminals) const;
  [[nodiscard]] bool
  loadEnergized(Dsu& dsu, const BaseGraph& base, const Component& component) const;
  [[nodiscard]] std::map<std::string, bool> groupStates() const;

  [[nodiscard]] bool advanceTimers();
  void evaluateLogic(BaseGraph& base);
  Result<Dsu> solveCoils(const BaseGraph& base);
  void updateLoads(Dsu& final_dsu, const BaseGraph& base);
  void updateVisualState(Dsu& final_dsu,
                         const BaseGraph& base,
                         const std::map<std::string, bool>& group_states);
  void recomputeLiveWires(Dsu& final_dsu, const BaseGraph& base);
  void logTransition(const ComponentState& previous,
                     const ComponentState& current,
                     const Component& component);
  void logInput(ComponentId component_id, std::string state, std::string value);
  void emitFrame();
  void appendLog(LogEntry entry);
  [[nodiscard]] bool initialClosed(const Component& component,
                                   const DigitalInputState& input) const;

  [[nodiscard]] const Component* component(ComponentId id) const;
  [[nodiscard]] DigitalInputState inputFor(ComponentId id) const;
  [[nodiscard]] bool isContactorCoilComponent(const Component& component) const;
  [[nodiscard]] std::int64_t delayMs(const Component& component) const;

  Diagram diagram_;
  SimulationOptions options_;
  std::map<ComponentId, ComponentState> states_;
  std::map<ComponentId, DigitalInputState> inputs_;
  std::set<WireId> live_wires_;
  std::vector<LogEntry> log_;
  std::int64_t time_ms_ = 0;
  std::uint64_t cycle_ = 0;
  std::optional<Error> fault_;
  SimulationObserver* observer_ = nullptr;
};

} // namespace fiovivo::sim
