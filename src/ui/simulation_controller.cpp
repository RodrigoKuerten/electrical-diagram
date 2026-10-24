#include "ui/simulation_controller.hpp"

#include "ui/simulation_worker.hpp"

#include <QMetaObject>
#include <QMetaType>

#include <algorithm>
#include <utility>

namespace fiovivo::ui {

SimulationController::SimulationController(QObject* parent) : QObject(parent) {
  qRegisterMetaType<SimFrame>("fiovivo::ui::SimFrame");
  qRegisterMetaType<SimLogEntry>("fiovivo::ui::SimLogEntry");
  qRegisterMetaType<SimulationController::State>("fiovivo::ui::SimulationController::State");
}

SimulationController::~SimulationController() { teardown(); }

void SimulationController::loadDiagram(const Diagram& diagram, int cycle_ms, double speed) {
  teardown();
  cycle_ms_ = std::max(1, cycle_ms);
  speed_ = std::clamp(speed, 0.1, 10.0);
  time_ms_ = 0;
  thread_ = std::make_unique<QThread>();
  worker_ = std::make_unique<SimulationWorker>(diagram, cycle_ms_, speed_);
  worker_->moveToThread(thread_.get());

  connect(
      worker_.get(), &SimulationWorker::frameProduced, this, &SimulationController::forwardFrame);
  connect(worker_.get(), &SimulationWorker::logProduced, this, &SimulationController::forwardLog);
  connect(worker_.get(), &SimulationWorker::faultRaised, this, &SimulationController::faultRaised);
  connect(
      worker_.get(), &SimulationWorker::resetOccurred, this, &SimulationController::resetOccurred);
  connect(worker_.get(), &SimulationWorker::runningChanged, this, [this](bool running) {
    if (running) {
      setState(State::Running);
    } else if (state_ == State::Running) {
      if (state_ != State::Stopped) {
        setState(State::Paused);
      }
    }
  });

  thread_->start();
  QMetaObject::invokeMethod(worker_.get(), "initialize", Qt::QueuedConnection);
  has_diagram_ = true;
  setState(State::Stopped);
}

void SimulationController::start() {
  if (!has_diagram_ || state_ == State::Running) {
    return;
  }
  ensureWorker();
  setState(State::Running);
  QMetaObject::invokeMethod(worker_.get(), "start", Qt::QueuedConnection);
}

void SimulationController::pause() {
  if (state_ != State::Running || worker_ == nullptr) {
    return;
  }
  QMetaObject::invokeMethod(worker_.get(), "pause", Qt::QueuedConnection);
  setState(State::Paused);
}

void SimulationController::toggleRunPause() {
  if (state_ == State::Running) {
    pause();
  } else {
    start();
  }
}

void SimulationController::stepOnce() {
  if (!has_diagram_) {
    return;
  }
  ensureWorker();
  if (state_ == State::Running) {
    pause();
  }
  QMetaObject::invokeMethod(worker_.get(), "stepOnce", Qt::QueuedConnection);
  setState(State::Paused);
}

void SimulationController::stepCycles(int count) {
  if (!has_diagram_) {
    return;
  }
  ensureWorker();
  if (state_ == State::Running) {
    pause();
  }
  QMetaObject::invokeMethod(worker_.get(), "stepCycles", Qt::QueuedConnection, Q_ARG(int, count));
  setState(State::Paused);
}

void SimulationController::resetSimulation() {
  if (worker_ == nullptr) {
    return;
  }
  QMetaObject::invokeMethod(worker_.get(), "resetSimulation", Qt::QueuedConnection);
  time_ms_ = 0;
  setState(State::Stopped);
  emit simulatedTimeChanged(0);
}

void SimulationController::setInput(quint64 component, bool active) {
  if (worker_ == nullptr) {
    return;
  }
  QMetaObject::invokeMethod(worker_.get(),
                            "setInput",
                            Qt::QueuedConnection,
                            Q_ARG(quint64, component),
                            Q_ARG(bool, active));
}

void SimulationController::setTripped(quint64 component, bool tripped) {
  if (worker_ == nullptr) {
    return;
  }
  QMetaObject::invokeMethod(worker_.get(),
                            "setTripped",
                            Qt::QueuedConnection,
                            Q_ARG(quint64, component),
                            Q_ARG(bool, tripped));
}

void SimulationController::setSelector(quint64 component, int position) {
  if (worker_ == nullptr) {
    return;
  }
  QMetaObject::invokeMethod(worker_.get(),
                            "setSelector",
                            Qt::QueuedConnection,
                            Q_ARG(quint64, component),
                            Q_ARG(int, position));
}

void SimulationController::setSpeed(double speed) {
  speed_ = std::clamp(speed, 0.1, 10.0);
  if (worker_ != nullptr) {
    QMetaObject::invokeMethod(
        worker_.get(), "setSpeed", Qt::QueuedConnection, Q_ARG(double, speed_));
  }
}

void SimulationController::discard() {
  teardown();
  has_diagram_ = false;
  time_ms_ = 0;
  setState(State::Stopped);
}

void SimulationController::setState(State state, bool notify) {
  if (state_ == state) {
    return;
  }
  state_ = state;
  if (notify) {
    emit stateChanged(state_);
  }
}

void SimulationController::teardown() {
  if (worker_ != nullptr) {
    if (thread_ != nullptr && thread_->isRunning()) {
      QMetaObject::invokeMethod(worker_.get(), "shutdown", Qt::BlockingQueuedConnection);
      thread_->quit();
      thread_->wait(3000);
    }
    worker_.reset();
  }
  if (thread_ != nullptr) {
    if (thread_->isRunning()) {
      thread_->quit();
      thread_->wait(3000);
    }
    thread_.reset();
  }
  if (state_ != State::Stopped) {
    setState(State::Stopped);
  }
}

void SimulationController::ensureWorker() {
  if (worker_ != nullptr || !has_diagram_) {
    return;
  }
  loadDiagram(Diagram{}, cycle_ms_, speed_);
}

void SimulationController::forwardFrame(const SimFrame& frame) {
  time_ms_ = frame.time_ms;
  emit simulatedTimeChanged(time_ms_);
  emit frameReady(frame);
}

void SimulationController::forwardLog(const SimLogEntry& entry) { emit logReady(entry); }

} // namespace fiovivo::ui
