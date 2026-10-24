#pragma once

#include "core/model.hpp"
#include "ui/sim_ui_types.hpp"

#include <QObject>
#include <QThread>

#include <memory>

namespace fiovivo::ui {

class SimulationWorker;

class SimulationController : public QObject {
  Q_OBJECT

public:
  enum class State { Stopped, Running, Paused };
  Q_ENUM(State)

  explicit SimulationController(QObject* parent = nullptr);
  ~SimulationController() override;

  void loadDiagram(const Diagram& diagram, int cycle_ms, double speed);
  [[nodiscard]] bool isLoaded() const { return worker_ != nullptr; }
  [[nodiscard]] State state() const { return state_; }
  [[nodiscard]] qint64 simulatedTimeMs() const { return time_ms_; }
  [[nodiscard]] double speed() const { return speed_; }

public slots:
  void start();
  void pause();
  void toggleRunPause();
  void stepOnce();
  void stepCycles(int count);
  void resetSimulation();
  void setInput(quint64 component, bool active);
  void setTripped(quint64 component, bool tripped);
  void setSelector(quint64 component, int position);
  void setSpeed(double speed);
  void discard();

signals:
  void frameReady(fiovivo::ui::SimFrame frame);
  void logReady(fiovivo::ui::SimLogEntry entry);
  void stateChanged(fiovivo::ui::SimulationController::State state);
  void faultRaised(const QString& message);
  void resetOccurred();
  void simulatedTimeChanged(qint64 time_ms);

private:
  void setState(State state, bool notify = true);
  void teardown();
  void ensureWorker();
  void forwardFrame(const SimFrame& frame);
  void forwardLog(const SimLogEntry& entry);

  std::unique_ptr<QThread> thread_;
  std::unique_ptr<SimulationWorker> worker_;
  State state_ = State::Stopped;
  int cycle_ms_ = 10;
  double speed_ = 1.0;
  qint64 time_ms_ = 0;
  bool has_diagram_ = false;
};

} // namespace fiovivo::ui

Q_DECLARE_METATYPE(fiovivo::ui::SimulationController::State)
