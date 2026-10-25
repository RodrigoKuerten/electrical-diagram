#pragma once

#include "core/ids.hpp"
#include "core/result.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace fiovivo::sim {

struct SimulationOptions {
  std::int64_t cycle_ms = 10;
  int max_passes = 16;
  std::size_t max_log_entries = 20000;

  friend bool operator==(const SimulationOptions&, const SimulationOptions&) = default;
};

struct ComponentState {
  bool energized = false;
  bool closed = false;
  bool timer_output = false;
  std::int64_t timer_elapsed_ms = 0;
  int selector_position = 1;
  bool input_active = false;
  bool tripped = false;
  bool last_energized = false;
  int counter = 0;
  bool logic_output = false;
  bool last_input = false;

  friend bool operator==(const ComponentState&, const ComponentState&) = default;
};

struct DigitalInputState {
  bool active = false;
  bool tripped = false;
  int selector_position = 1;

  friend bool operator==(const DigitalInputState&, const DigitalInputState&) = default;
};

struct SimulationFrame {
  std::int64_t time_ms = 0;
  std::uint64_t cycle = 0;
  std::map<ComponentId, ComponentState> states;
  std::set<WireId> live_wires;
};

struct LogEntry {
  std::int64_t time_ms = 0;
  ComponentId component;
  std::string tag;
  std::string state;
  std::string value;

  friend bool operator==(const LogEntry&, const LogEntry&) = default;
};

struct SimulationSnapshot {
  std::int64_t time_ms = 0;
  std::uint64_t cycle = 0;
  std::map<ComponentId, DigitalInputState> inputs;
  std::map<ComponentId, ComponentState> states;

  friend bool operator==(const SimulationSnapshot&, const SimulationSnapshot&) = default;
};

class SimulationObserver {
public:
  virtual ~SimulationObserver() = default;
  virtual void onFrame(const SimulationFrame& frame) = 0;
  virtual void onLog(const LogEntry& entry) = 0;
  virtual void onFault(const Error& error) = 0;
};

} // namespace fiovivo::sim
