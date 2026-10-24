#include "ui/app_settings.hpp"

#include "ui/theme.hpp"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QSettings>
#include <QStandardPaths>

#include <memory>

namespace fiovivo::ui {

class AppSettings::Impl {
public:
  Impl() {
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(directory);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory);
    settings = std::make_unique<QSettings>(QSettings::IniFormat,
                                           QSettings::UserScope,
                                           QStringLiteral("fiovivo"),
                                           QStringLiteral("settings"));
  }

  std::unique_ptr<QSettings> settings;
};

AppSettings::AppSettings() : impl_(new Impl()) {}
AppSettings::~AppSettings() { delete impl_; }

QString AppSettings::language() const {
  return impl_->settings->value("ui/language", "pt_BR").toString();
}
void AppSettings::setLanguage(const QString& code) {
  impl_->settings->setValue("ui/language", code);
  impl_->settings->sync();
}

bool AppSettings::gridVisible() const {
  return impl_->settings->value("canvas/grid_visible", true).toBool();
}
void AppSettings::setGridVisible(bool visible) {
  impl_->settings->setValue("canvas/grid_visible", visible);
}

double AppSettings::gridSize() const {
  return impl_->settings->value("canvas/grid_size", 5.0).toDouble();
}
void AppSettings::setGridSize(double size) {
  impl_->settings->setValue("canvas/grid_size", qBound(1.0, size, 50.0));
}

int AppSettings::cycleMs() const {
  return impl_->settings->value("simulation/cycle_ms", 10).toInt();
}
void AppSettings::setCycleMs(int cycle_ms) {
  impl_->settings->setValue("simulation/cycle_ms", qBound(1, cycle_ms, 1000));
}

double AppSettings::simulationSpeed() const {
  return impl_->settings->value("simulation/speed", 1.0).toDouble();
}
void AppSettings::setSimulationSpeed(double speed) {
  impl_->settings->setValue("simulation/speed", qBound(0.1, speed, 10.0));
}

QString AppSettings::theme() const {
  return themeModeName(themeModeFromName(impl_->settings->value("ui/theme", "light").toString()));
}
void AppSettings::setTheme(const QString& theme) {
  impl_->settings->setValue("ui/theme", themeModeName(themeModeFromName(theme)));
}

int AppSettings::fontPointSize() const {
  return impl_->settings->value("ui/font_point_size", 10).toInt();
}
void AppSettings::setFontPointSize(int points) {
  impl_->settings->setValue("ui/font_point_size", qBound(7, points, 24));
}

QString AppSettings::lastDirectory() const {
  return impl_->settings->value("files/last_directory").toString();
}
void AppSettings::setLastDirectory(const QString& directory) {
  impl_->settings->setValue("files/last_directory", directory);
}

QStringList AppSettings::recentFiles() const {
  return impl_->settings->value("files/recent").toStringList();
}
void AppSettings::addRecentFile(const QString& path) {
  QStringList files = recentFiles();
  files.removeAll(path);
  files.prepend(path);
  while (files.size() > 10) {
    files.removeLast();
  }
  impl_->settings->setValue("files/recent", files);
  impl_->settings->sync();
}

QByteArray AppSettings::windowGeometry() const {
  return impl_->settings->value("window/geometry").toByteArray();
}
void AppSettings::setWindowGeometry(const QByteArray& geometry) {
  impl_->settings->setValue("window/geometry", geometry);
}

QByteArray AppSettings::windowState() const {
  return impl_->settings->value("window/state").toByteArray();
}
void AppSettings::setWindowState(const QByteArray& state) {
  impl_->settings->setValue("window/state", state);
}

void AppSettings::applyAppearance(QApplication& application) const {
  QFont font = application.font();
  font.setPointSize(fontPointSize());
  application.setFont(font);
  (void)applyTheme(application, themeModeFromName(theme()));
}

} // namespace fiovivo::ui
