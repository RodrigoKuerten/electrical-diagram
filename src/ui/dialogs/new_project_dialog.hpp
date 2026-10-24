#pragma once

#include "core/types.hpp"

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;

namespace fiovivo::ui {

struct NewProjectOptions {
  QString title;
  QString author;
  PaperSize paper = PaperSize::A4;
  Orientation orientation = Orientation::Landscape;
  QString template_path;
};

class NewProjectDialog : public QDialog {
  Q_OBJECT

public:
  explicit NewProjectDialog(const QString& examples_directory, QWidget* parent = nullptr);

  [[nodiscard]] NewProjectOptions options() const;

private:
  QLineEdit* title_ = nullptr;
  QLineEdit* author_ = nullptr;
  QComboBox* paper_ = nullptr;
  QComboBox* orientation_ = nullptr;
  QComboBox* template_ = nullptr;
};

} // namespace fiovivo::ui
