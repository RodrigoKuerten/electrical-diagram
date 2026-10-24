#include "ui/simulation_log_dock.hpp"

#include "ui/ui_helpers.hpp"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStringConverter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

#include <algorithm>

namespace fiovivo::ui {
namespace {

[[nodiscard]] QString csvField(const QString& value) {
  QString escaped = value;
  escaped.replace('"', "\"\"");
  return '"' + escaped + '"';
}

} // namespace

SimulationLogDock::SimulationLogDock(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  auto* buttons = new QHBoxLayout();
  auto* clear_button = new QPushButton(tr("Clear Log"), this);
  auto* export_button = new QPushButton(tr("Export CSV..."), this);
  buttons->addWidget(clear_button);
  buttons->addWidget(export_button);
  buttons->addStretch(1);
  summary_ = new QLabel(tr("No events yet."), this);
  buttons->addWidget(summary_);
  layout->addLayout(buttons);
  table_ = new QTableWidget(this);
  table_->setColumnCount(4);
  table_->setHorizontalHeaderLabels({tr("Time"), tr("Component"), tr("State"), tr("Value")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  layout->addWidget(table_, 1);
  connect(clear_button, &QPushButton::clicked, this, &SimulationLogDock::clearRequested);
  connect(clear_button, &QPushButton::clicked, this, &SimulationLogDock::clear);
  connect(export_button, &QPushButton::clicked, this, [this] {
    const QString path = QFileDialog::getSaveFileName(this,
                                                      tr("Export Simulation Log"),
                                                      "simulation-log.csv",
                                                      tr("CSV files (*.csv);;All files (*)"));
    if (path.isEmpty()) {
      return;
    }
    if (exportCsv(path)) {
      emit statusMessage(tr("Simulation log exported to %1").arg(path));
    } else {
      emit statusMessage(tr("Could not export the simulation log."));
    }
  });
}

QString SimulationLogDock::stateText(const QString& state) const {
  if (state == "coil") {
    return tr("Coil");
  }
  if (state == "contact") {
    return tr("Contact");
  }
  if (state == "timer") {
    return tr("Timer");
  }
  if (state == "lamp") {
    return tr("Lamp");
  }
  if (state == "motor") {
    return tr("Motor");
  }
  if (state == "emergency_stop") {
    return tr("Emergency stop");
  }
  if (state == "input") {
    return tr("Input");
  }
  if (state == "tripped") {
    return tr("Protection");
  }
  if (state == "position") {
    return tr("Selector position");
  }
  return state;
}

QString SimulationLogDock::valueText(const QString& state, const QString& value) const {
  if (value == "on") {
    return tr("On");
  }
  if (value == "off") {
    return tr("Off");
  }
  if (value == "closed") {
    return tr("Closed");
  }
  if (value == "open") {
    return tr("Open");
  }
  if (value == "active") {
    return tr("Active");
  }
  if (value == "rest") {
    return tr("Rest");
  }
  if (value == "running") {
    return tr("Running");
  }
  if (value == "stopped") {
    return tr("Stopped");
  }
  if (value == "triggered") {
    return tr("Triggered");
  }
  if (value == "reset") {
    return tr("Reset");
  }
  if (state == "position") {
    return tr("Position %1").arg(value);
  }
  return value;
}

void SimulationLogDock::appendEntry(const SimLogEntry& entry) {
  entries_.push_back(entry);
  table_->insertRow(table_->rowCount());
  const int row = table_->rowCount() - 1;
  auto* time = new QTableWidgetItem(QString::number(entry.time_ms));
  auto* tag = new QTableWidgetItem(entry.tag);
  auto* state = new QTableWidgetItem(stateText(entry.state));
  auto* value = new QTableWidgetItem(valueText(entry.state, entry.value));
  table_->setItem(row, 0, time);
  table_->setItem(row, 1, tag);
  table_->setItem(row, 2, state);
  table_->setItem(row, 3, value);
  if (entries_.size() > static_cast<std::size_t>(kMaxEntries)) {
    entries_.erase(entries_.begin());
    table_->removeRow(0);
  }
  table_->scrollToBottom();
  summary_->setText(tr("%1 event(s)").arg(static_cast<qulonglong>(entries_.size())));
}

void SimulationLogDock::clear() {
  entries_.clear();
  table_->setRowCount(0);
  summary_->setText(tr("No events yet."));
}

bool SimulationLogDock::exportCsv(const QString& path) const {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  stream << "\"time_ms\",\"tag\",\"state\",\"value\"\n";
  for (const SimLogEntry& entry : entries_) {
    stream << entry.time_ms << ',' << csvField(entry.tag) << ',' << csvField(entry.state) << ','
           << csvField(entry.value) << '\n';
  }
  return true;
}

} // namespace fiovivo::ui
