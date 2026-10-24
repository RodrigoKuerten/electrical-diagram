#include "ui/dialogs/preferences_dialog.hpp"

#include "ui/app_settings.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

namespace fiovivo::ui {

PreferencesDialog::PreferencesDialog(AppSettings& settings, QWidget* parent)
    : QDialog(parent), settings_(settings) {
  setWindowTitle(tr("Preferences"));
  setMinimumWidth(460);
  auto* layout = new QVBoxLayout(this);
  auto* form = new QFormLayout();
  language_ = new QComboBox(this);
  language_->addItem(tr("Português (Brasil)"), QStringLiteral("pt_BR"));
  language_->addItem(tr("English"), QStringLiteral("en"));
  language_->setCurrentIndex(language_->findData(settings_.language()));

  grid_visible_ = new QCheckBox(tr("Show grid"), this);
  grid_visible_->setChecked(settings_.gridVisible());
  grid_size_ = new QDoubleSpinBox(this);
  grid_size_->setRange(1.0, 50.0);
  grid_size_->setDecimals(1);
  grid_size_->setSuffix(QStringLiteral(" mm"));
  grid_size_->setValue(settings_.gridSize());

  cycle_ms_ = new QSpinBox(this);
  cycle_ms_->setRange(1, 1000);
  cycle_ms_->setSuffix(QStringLiteral(" ms"));
  cycle_ms_->setValue(settings_.cycleMs());

  speed_ = new QComboBox(this);
  for (double value : {0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0}) {
    speed_->addItem(QString::number(value, 'g', 2) + "x", value);
  }
  speed_->setCurrentIndex(std::max(0, speed_->findData(settings_.simulationSpeed())));

  theme_ = new QComboBox(this);
  theme_->addItem(tr("Light"), QStringLiteral("light"));
  theme_->addItem(tr("Dark"), QStringLiteral("dark"));
  theme_->setCurrentIndex(std::max(0, theme_->findData(settings_.theme())));

  font_size_ = new QSpinBox(this);
  font_size_->setRange(7, 24);
  font_size_->setSuffix(tr(" pt"));
  font_size_->setValue(settings_.fontPointSize());

  form->addRow(tr("Language"), language_);
  form->addRow(tr("Grid"), grid_visible_);
  form->addRow(tr("Grid size"), grid_size_);
  form->addRow(tr("Simulation cycle"), cycle_ms_);
  form->addRow(tr("Default speed"), speed_);
  form->addRow(tr("Theme"), theme_);
  form->addRow(tr("Font size"), font_size_);
  layout->addLayout(form);

  auto* hint =
      new QLabel(tr("Language changes are applied after restarting the application."), this);
  hint->setWordWrap(true);
  layout->addWidget(hint);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, [this] {
    const QString old_language = settings_.language();
    settings_.setLanguage(language_->currentData().toString());
    settings_.setGridVisible(grid_visible_->isChecked());
    settings_.setGridSize(grid_size_->value());
    settings_.setCycleMs(cycle_ms_->value());
    settings_.setSimulationSpeed(speed_->currentData().toDouble());
    settings_.setTheme(theme_->currentData().toString());
    settings_.setFontPointSize(font_size_->value());
    if (old_language != language_->currentData().toString()) {
      QMessageBox::information(this,
                               tr("Restart required"),
                               tr("The language will change the next time FioVivo starts."));
    }
    accept();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

} // namespace fiovivo::ui
