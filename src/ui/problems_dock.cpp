#include "ui/problems_dock.hpp"

#include "ui/ui_helpers.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <string>

namespace fiovivo::ui {
namespace {

[[nodiscard]] QString severityText(ProblemSeverity severity) {
  switch (severity) {
  case ProblemSeverity::Error:
    return ProblemsDock::tr("Error");
  case ProblemSeverity::Warning:
    return ProblemsDock::tr("Warning");
  case ProblemSeverity::Info:
    return ProblemsDock::tr("Info");
  }
  return {};
}

[[nodiscard]] QString codeText(ProblemCode code) {
  switch (code) {
  case ProblemCode::DuplicateTag:
    return ProblemsDock::tr("Duplicate tag");
  case ProblemCode::DuplicateId:
    return ProblemsDock::tr("Duplicate identifier");
  case ProblemCode::FloatingWire:
    return ProblemsDock::tr("Floating wire");
  case ProblemCode::InvalidWireEndpoint:
    return ProblemsDock::tr("Invalid wire endpoint");
  case ProblemCode::SelfLoopNotAllowed:
    return ProblemsDock::tr("Self-loop not allowed");
  case ProblemCode::RequiredPinUnconnected:
    return ProblemsDock::tr("Required pin unconnected");
  case ProblemCode::UnmatchedContact:
    return ProblemsDock::tr("Unmatched contact");
  case ProblemCode::CoilWithoutContact:
    return ProblemsDock::tr("Coil without contacts");
  case ProblemCode::MissingSupplyPath:
    return ProblemsDock::tr("Missing supply path");
  case ProblemCode::ComponentOverlap:
    return ProblemsDock::tr("Overlapping components");
  case ProblemCode::InvalidComponent:
    return ProblemsDock::tr("Invalid component");
  case ProblemCode::InvalidPinConnection:
    return ProblemsDock::tr("Invalid pin connection");
  }
  return {};
}

} // namespace

ProblemsDock::ProblemsDock(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  summary_ = new QLabel(this);
  summary_->setWordWrap(true);
  layout->addWidget(summary_);
  table_ = new QTableWidget(this);
  table_->setColumnCount(3);
  table_->setHorizontalHeaderLabels({tr("Severity"), tr("Problem"), tr("Details")});
  table_->horizontalHeader()->setStretchLastSection(true);
  table_->verticalHeader()->setVisible(false);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::SingleSelection);
  layout->addWidget(table_, 1);
  connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
    (void)column;
    if (row < 0 || row >= static_cast<int>(problems_.size())) {
      return;
    }
    const Problem& problem = problems_[static_cast<std::size_t>(row)];
    emit navigateTo(problem.component.value_or(ComponentId{}).value(),
                    problem.component.has_value(),
                    problem.wire.value_or(WireId{}).value());
  });
  refreshSummary();
}

void ProblemsDock::setReport(const ValidationReport& report) {
  problems_ = report.problems;
  table_->setRowCount(static_cast<int>(problems_.size()));
  for (int row = 0; row < static_cast<int>(problems_.size()); ++row) {
    const Problem& problem = problems_[static_cast<std::size_t>(row)];
    auto* severity = new QTableWidgetItem(severityText(problem.severity));
    switch (problem.severity) {
    case ProblemSeverity::Error:
      severity->setIcon(style()->standardIcon(QStyle::SP_MessageBoxCritical));
      break;
    case ProblemSeverity::Warning:
      severity->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
      break;
    case ProblemSeverity::Info:
      severity->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
      break;
    }
    auto* code = new QTableWidgetItem(codeText(problem.code));
    auto* message = new QTableWidgetItem(QString::fromStdString(problem.message));
    message->setToolTip(QString::fromStdString(problem.message));
    table_->setItem(row, 0, severity);
    table_->setItem(row, 1, code);
    table_->setItem(row, 2, message);
  }
  table_->resizeColumnsToContents();
  refreshSummary();
}

void ProblemsDock::clear() {
  problems_.clear();
  table_->setRowCount(0);
  refreshSummary();
}

void ProblemsDock::refreshSummary() {
  const std::size_t errors = static_cast<std::size_t>(
      std::count_if(problems_.begin(), problems_.end(), [](const Problem& problem) {
        return problem.severity == ProblemSeverity::Error;
      }));
  const std::size_t warnings = static_cast<std::size_t>(
      std::count_if(problems_.begin(), problems_.end(), [](const Problem& problem) {
        return problem.severity == ProblemSeverity::Warning;
      }));
  const std::size_t infos = problems_.size() - errors - warnings;
  summary_->setText(
      tr("%1 error(s), %2 warning(s), %3 information item(s). Double-click a row to locate it.")
          .arg(static_cast<qulonglong>(errors))
          .arg(static_cast<qulonglong>(warnings))
          .arg(static_cast<qulonglong>(infos)));
  summary_->setStyleSheet(errors > 0
                              ? QStringLiteral("color:#b91c1c;")
                              : (warnings > 0 ? QStringLiteral("color:#b45309;") : QString()));
}

} // namespace fiovivo::ui
