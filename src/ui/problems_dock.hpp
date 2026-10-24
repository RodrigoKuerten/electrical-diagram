#pragma once

#include "core/validation.hpp"
#include "ui/sim_ui_types.hpp"

#include <QWidget>

#include <vector>

class QTableWidget;
class QLabel;

namespace fiovivo::ui {

class ProblemsDock : public QWidget {
  Q_OBJECT

public:
  explicit ProblemsDock(QWidget* parent = nullptr);

  void setReport(const ValidationReport& report);
  void clear();
  void refreshTheme();

signals:
  void navigateTo(quint64 component, bool has_component, quint64 wire);

private:
  void refreshSummary();

  QTableWidget* table_ = nullptr;
  QLabel* summary_ = nullptr;
  std::vector<Problem> problems_;
};

} // namespace fiovivo::ui
