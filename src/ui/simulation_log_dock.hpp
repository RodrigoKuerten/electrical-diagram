#pragma once

#include "ui/sim_ui_types.hpp"

#include <QWidget>

#include <vector>

class QTableWidget;
class QLabel;

namespace fiovivo::ui {

class SimulationLogDock : public QWidget {
  Q_OBJECT

public:
  explicit SimulationLogDock(QWidget* parent = nullptr);

  void appendEntry(const SimLogEntry& entry);
  void clear();
  [[nodiscard]] std::size_t entryCount() const { return entries_.size(); }
  [[nodiscard]] bool exportCsv(const QString& path) const;

signals:
  void statusMessage(const QString& message);
  void clearRequested();

private:
  [[nodiscard]] QString stateText(const QString& state) const;
  [[nodiscard]] QString valueText(const QString& state, const QString& value) const;

  QTableWidget* table_ = nullptr;
  QLabel* summary_ = nullptr;
  std::vector<SimLogEntry> entries_;
  static constexpr int kMaxEntries = 5000;
};

} // namespace fiovivo::ui
