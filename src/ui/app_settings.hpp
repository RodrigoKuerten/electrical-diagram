#pragma once

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QStringList>

class QApplication;

namespace fiovivo::ui {

class AppSettings {
public:
  AppSettings();
  ~AppSettings();

  [[nodiscard]] QString language() const;
  void setLanguage(const QString& code);

  [[nodiscard]] bool gridVisible() const;
  void setGridVisible(bool visible);
  [[nodiscard]] double gridSize() const;
  void setGridSize(double size);

  [[nodiscard]] QColor wireColor() const;
  void setWireColor(const QColor& color);
  [[nodiscard]] QColor energizedColor() const;
  void setEnergizedColor(const QColor& color);

  [[nodiscard]] int cycleMs() const;
  void setCycleMs(int cycle_ms);
  [[nodiscard]] double simulationSpeed() const;
  void setSimulationSpeed(double speed);

  [[nodiscard]] QString theme() const;
  void setTheme(const QString& theme);
  [[nodiscard]] int fontPointSize() const;
  void setFontPointSize(int points);

  [[nodiscard]] QString lastDirectory() const;
  void setLastDirectory(const QString& directory);
  [[nodiscard]] QStringList recentFiles() const;
  void addRecentFile(const QString& path);

  [[nodiscard]] QByteArray windowGeometry() const;
  void setWindowGeometry(const QByteArray& geometry);
  [[nodiscard]] QByteArray windowState() const;
  void setWindowState(const QByteArray& state);

  void applyAppearance(QApplication& application) const;

private:
  class Impl;
  Impl* impl_ = nullptr;
  AppSettings(const AppSettings&) = delete;
  AppSettings& operator=(const AppSettings&) = delete;
};

} // namespace fiovivo::ui
