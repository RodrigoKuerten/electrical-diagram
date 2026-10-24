#include "ui/simulation_worker.hpp"

#include "core/ids.hpp"

#include <QTimer>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace fiovivo::ui {

SimulationWorker::SimulationWorker(Diagram diagram, int cycle_ms, double speed, QObject* parent)
    : QObject(parent), cycle_ms_(std::max(1, cycle_ms)), speed_(std::clamp(speed, 0.1, 10.0)) {
  simulator_ = std::make_unique<sim::Simulator>(std::move(diagram),
                                                sim::SimulationOptions{cycle_ms_, 16, 50000});
}

SimulationWorker::~SimulationWorker() = default;

void SimulationWorker::initialize() {
  if (initialized_) {
    return;
  }
  initialized_ = true;
  simulator_->setObserver(this);
  timer_ = std::make_unique<QTimer>();
  connect(timer_.get(), &QTimer::timeout, this, &SimulationWorker::tick);
  emit frameProduced(makeFrame());
  emit resetOccurred();
}

void SimulationWorker::start() {
  if (running_) {
    return;
  }
  if (simulator_->hasFault()) {
    emit faultRaised(QString::fromStdString(simulator_->fault()->toString()));
    return;
  }
  if (timer_ == nullptr) {
    initialize();
  }
  accumulator_ms_ = 0.0;
  tick_clock_.start();
  timer_->start(timerIntervalMs());
  running_ = true;
  emit runningChanged(true);
  flushFrame();
}

void SimulationWorker::pause() {
  if (!running_) {
    return;
  }
  running_ = false;
  if (timer_ != nullptr) {
    timer_->stop();
  }
  flushFrame();
  emit runningChanged(false);
}

void SimulationWorker::stepOnce() {
  if (running_) {
    pause();
  }
  if (simulator_->hasFault()) {
    emit faultRaised(QString::fromStdString(simulator_->fault()->toString()));
    return;
  }
  const auto result = simulator_->step();
  if (!result) {
    handleFault(result.error());
    return;
  }
  flushFrame();
}

void SimulationWorker::stepCycles(int count) {
  if (running_) {
    pause();
  }
  const int bounded = std::clamp(count, 1, 100000);
  for (int index = 0; index < bounded; ++index) {
    if (simulator_->hasFault()) {
      break;
    }
    const auto result = simulator_->step();
    if (!result) {
      handleFault(result.error());
      break;
    }
  }
  flushFrame();
}

void SimulationWorker::resetSimulation() {
  const bool was_running = running_;
  running_ = false;
  if (timer_ != nullptr) {
    timer_->stop();
  }
  accumulator_ms_ = 0.0;
  const auto result = simulator_->reset();
  if (!result) {
    handleFault(result.error());
    return;
  }
  emit resetOccurred();
  flushFrame();
  if (was_running) {
    emit runningChanged(false);
  }
}

void SimulationWorker::setInput(quint64 component, bool active) {
  const auto result = simulator_->setInput(ComponentId{component}, active);
  if (!result) {
    emit faultRaised(QString::fromStdString(result.error().toString()));
    return;
  }
  flushFrame();
}

void SimulationWorker::setTripped(quint64 component, bool tripped) {
  const auto result = simulator_->setTripped(ComponentId{component}, tripped);
  if (!result) {
    emit faultRaised(QString::fromStdString(result.error().toString()));
    return;
  }
  flushFrame();
}

void SimulationWorker::setSelector(quint64 component, int position) {
  const auto result = simulator_->setSelectorPosition(ComponentId{component}, position);
  if (!result) {
    emit faultRaised(QString::fromStdString(result.error().toString()));
    return;
  }
  flushFrame();
}

void SimulationWorker::setSpeed(double speed) {
  speed_ = std::clamp(speed, 0.1, 10.0);
  if (running_ && timer_ != nullptr) {
    accumulator_ms_ = 0.0;
    tick_clock_.restart();
    timer_->start(timerIntervalMs());
  }
}

void SimulationWorker::shutdown() {
  running_ = false;
  if (timer_ != nullptr) {
    timer_->stop();
    timer_.reset();
  }
  emit runningChanged(false);
}

void SimulationWorker::tick() {
  if (!running_) {
    return;
  }
  const qint64 elapsed = tick_clock_.restart();
  const double step_wall_ms = static_cast<double>(cycle_ms_) / speed_;
  accumulator_ms_ += static_cast<double>(elapsed);
  int steps = static_cast<int>(std::floor(accumulator_ms_ / step_wall_ms));
  steps = std::clamp(steps, 0, 500);
  if (steps > 0) {
    accumulator_ms_ -= static_cast<double>(steps) * step_wall_ms;
  }
  for (int index = 0; index < steps; ++index) {
    if (simulator_->hasFault()) {
      break;
    }
    const auto result = simulator_->step();
    if (!result) {
      handleFault(result.error());
      return;
    }
  }
  flushFrame();
}

void SimulationWorker::flushFrame() {
  if (simulator_ == nullptr) {
    return;
  }
  emit frameProduced(makeFrame());
}

SimFrame SimulationWorker::makeFrame() const {
  SimFrame frame;
  frame.time_ms = simulator_->simulatedTimeMs();
  frame.cycle = simulator_->cycleCount();
  for (const auto& [id, state] : simulator_->states()) {
    SimItemState item;
    item.energized = state.energized;
    item.closed = state.closed;
    item.timer_output = state.timer_output;
    item.timer_elapsed_ms = state.timer_elapsed_ms;
    item.selector_position = state.selector_position;
    item.input_active = state.input_active;
    item.tripped = state.tripped;
    frame.states.insert(id.value(), item);
  }
  for (const WireId wire : simulator_->liveWires()) {
    frame.live_wires.insert(wire.value());
  }
  return frame;
}

void SimulationWorker::handleFault(const Error& error) {
  running_ = false;
  if (timer_ != nullptr) {
    timer_->stop();
  }
  emit faultRaised(QString::fromStdString(error.toString()));
  flushFrame();
  emit runningChanged(false);
}

int SimulationWorker::timerIntervalMs() const {
  const double step_wall_ms = static_cast<double>(cycle_ms_) / speed_;
  const int interval = static_cast<int>(std::lround(step_wall_ms));
  return std::clamp(interval, step_wall_ms < 5.0 ? 5 : 1, 1000);
}

void SimulationWorker::onFrame(const sim::SimulationFrame& frame) {
  (void)frame;
  // Frames are built on demand at tick/step boundaries to avoid flooding the UI thread.
}

void SimulationWorker::onLog(const sim::LogEntry& entry) {
  emit logProduced(SimLogEntry{entry.time_ms,
                               entry.component.value(),
                               QString::fromStdString(entry.tag),
                               QString::fromStdString(entry.state),
                               QString::fromStdString(entry.value)});
}

void SimulationWorker::onFault(const Error& error) {
  (void)error;
  // Step() returns the same error, which is handled once by handleFault.
}

} // namespace fiovivo::ui
