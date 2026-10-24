#pragma once

#include <QDialog>

class QLabel;

namespace fiovivo::ui {

class AboutDialog : public QDialog {
  Q_OBJECT

public:
  explicit AboutDialog(QWidget* parent = nullptr);
};

} // namespace fiovivo::ui
