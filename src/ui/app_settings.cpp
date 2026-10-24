#include "ui/app_settings.hpp"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>

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

QColor AppSettings::wireColor() const {
  return QColor(impl_->settings->value("canvas/wire_color", "#1f2933").toString());
}
void AppSettings::setWireColor(const QColor& color) {
  impl_->settings->setValue("canvas/wire_color", color.name());
}

QColor AppSettings::energizedColor() const {
  return QColor(impl_->settings->value("canvas/energized_color", "#d97706").toString());
}
void AppSettings::setEnergizedColor(const QColor& color) {
  impl_->settings->setValue("canvas/energized_color", color.name());
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
  return impl_->settings->value("ui/theme", "light").toString();
}
void AppSettings::setTheme(const QString& theme) { impl_->settings->setValue("ui/theme", theme); }

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

  if (theme() == "dark") {
    application.setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#1e2329"));
    palette.setColor(QPalette::WindowText, QColor("#e7edf3"));
    palette.setColor(QPalette::Base, QColor("#161a1f"));
    palette.setColor(QPalette::AlternateBase, QColor("#20262d"));
    palette.setColor(QPalette::ToolTipBase, QColor("#2b333c"));
    palette.setColor(QPalette::ToolTipText, QColor("#e7edf3"));
    palette.setColor(QPalette::Text, QColor("#e7edf3"));
    palette.setColor(QPalette::Button, QColor("#242b33"));
    palette.setColor(QPalette::ButtonText, QColor("#e7edf3"));
    palette.setColor(QPalette::BrightText, QColor("#ff6b6b"));
    palette.setColor(QPalette::Link, QColor("#63b3ed"));
    palette.setColor(QPalette::Highlight, QColor("#2b6cb0"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#7b8794"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#7b8794"));
    application.setPalette(palette);
  } else {
    application.setStyle(QStyleFactory::create("Fusion"));
    application.setPalette(application.style()->standardPalette());
  }
}

} // namespace fiovivo::ui
