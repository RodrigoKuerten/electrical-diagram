#pragma once

#include "core/model.hpp"
#include "sim/simulation_types.hpp"
#include "sim/simulator.hpp"
#include "ui/sim_ui_types.hpp"

#include <QElapsedTimer>
#include <QObject>

#include <memory>

class QTimer;

namespace fiovivo::ui {

// Lives in a worker QThread. All public slots are invoked through queued connections. The
// simulator observes the model snapshot passed at construction and never touches widgets.
class SimulationWorker : public QObject, public sim::SimulationObserver {
  Q_OBJECT

public:
  SimulationWorker(Diagram diagram, int cycle_ms, double speed, QObject* parent = nullptr);
  ~SimulationWorker() override;

  SimulationWorker(const SimulationWorker&) = delete;
  SimulationWorker& operator=(const SimulationWorker&) = delete;

  void onFrame(const sim::SimulationFrame& frame) override;
  void onLog(const sim::LogEntry& entry) override;
  void onFault(const Error& error) override;

public slots:
  void initialize();
  void start();
  void pause();
  void stepOnce();
  void stepCycles(int count);
  void resetSimulation();
  void setInput(quint64 component, bool active);
  void setTripped(quint64 component, bool tripped);
  void setSelector(quint64 component, int position);
  void setSpeed(double speed);
  void shutdown();

signals:
  void frameProduced(fiovivo::ui::SimFrame frame);
  void logProduced(fiovivo::ui::SimLogEntry entry);
  void runningChanged(bool running);
  void resetOccurred();
  void faultRaised(QString message);

private:
  void tick();
  void flushFrame();
  void handleFault(const Error& error);
  [[nodiscard]] int timerIntervalMs() const;
  [[nodiscard]] SimFrame makeFrame() const;

  std::unique_ptr<sim::Simulator> simulator_;
  std::unique_ptr<QTimer> timer_;
  QElapsedTimer tick_clock_;
  double accumulator_ms_ = 0.0;
  int cycle_ms_ = 10;
  double speed_ = 1.0;
  bool running_ = false;
  bool initialized_ = false;
};

} // namespace fiovivo::ui
