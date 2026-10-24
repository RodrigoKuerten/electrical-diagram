#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;

namespace fiovivo::ui {

class AppSettings;

class PreferencesDialog : public QDialog {
  Q_OBJECT

public:
  explicit PreferencesDialog(AppSettings& settings, QWidget* parent = nullptr);

private:
  void chooseWireColor();
  void chooseEnergizedColor();

  AppSettings& settings_;
  QComboBox* language_ = nullptr;
  QCheckBox* grid_visible_ = nullptr;
  QDoubleSpinBox* grid_size_ = nullptr;
  QPushButton* wire_color_ = nullptr;
  QPushButton* energized_color_ = nullptr;
  QSpinBox* cycle_ms_ = nullptr;
  QComboBox* speed_ = nullptr;
  QComboBox* theme_ = nullptr;
  QSpinBox* font_size_ = nullptr;
};

} // namespace fiovivo::ui
