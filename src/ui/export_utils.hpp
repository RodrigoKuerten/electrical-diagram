#pragma once

#include <QString>

class QPrinter;

namespace fiovivo::ui {

class SheetScene;

struct PngExportOptions {
  int scale = 2;
  bool transparent = false;
};

[[nodiscard]] bool exportPng(SheetScene& scene,
                             const QString& path,
                             const PngExportOptions& options,
                             QString* error = nullptr);
[[nodiscard]] bool exportSvg(SheetScene& scene,
                             const QString& path,
                             bool transparent = false,
                             QString* error = nullptr);
[[nodiscard]] bool exportPdf(SheetScene& scene, const QString& path, QString* error = nullptr);
[[nodiscard]] bool printScene(SheetScene& scene, QPrinter& printer, QString* error = nullptr);

} // namespace fiovivo::ui
