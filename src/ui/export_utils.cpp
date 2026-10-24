#include "ui/export_utils.hpp"

#include "ui/sheet_scene.hpp"

#include <QGraphicsItem>
#include <QImage>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QSvgGenerator>

#include <cmath>
#include <exception>

namespace fiovivo::ui {
namespace {

constexpr double kPixelsPerMillimetreAt96Dpi = 3.7795275591;

class ExportSession {
public:
  explicit ExportSession(SheetScene& scene, bool transparent)
      : scene_(scene), selected_(scene.selectedItems()) {
    scene_.clearSelection();
    scene_.setExportMode(true);
    scene_.setExportBackgroundTransparent(transparent);
  }
  ~ExportSession() {
    scene_.setExportMode(false);
    scene_.setExportBackgroundTransparent(false);
    for (QGraphicsItem* item : selected_) {
      item->setSelected(true);
    }
  }
  ExportSession(const ExportSession&) = delete;
  ExportSession& operator=(const ExportSession&) = delete;

private:
  SheetScene& scene_;
  QList<QGraphicsItem*> selected_;
};

[[nodiscard]] bool validatePage(const SheetScene& scene, QString* error) {
  if (!scene.pageRect().isValid()) {
    if (error != nullptr) {
      *error = QObject::tr("The sheet has no valid page size.");
    }
    return false;
  }
  return true;
}

} // namespace

bool exportPng(SheetScene& scene,
               const QString& path,
               const PngExportOptions& options,
               QString* error) {
  if (!validatePage(scene, error)) {
    return false;
  }
  try {
    ExportSession session(scene, options.transparent);
    const QRectF page = scene.pageRect();
    const double factor = kPixelsPerMillimetreAt96Dpi * std::max(1, options.scale);
    const QSize pixel_size(static_cast<int>(std::ceil(page.width() * factor)),
                           static_cast<int>(std::ceil(page.height() * factor)));
    QImage image(pixel_size, QImage::Format_ARGB32_Premultiplied);
    image.fill(options.transparent ? Qt::transparent : Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    scene.render(&painter, QRectF(QPointF(0.0, 0.0), QSizeF(pixel_size)), page);
    painter.end();
    if (!image.save(path, "PNG")) {
      if (error != nullptr) {
        *error = QObject::tr("Could not write the PNG file.");
      }
      return false;
    }
    return true;
  } catch (const std::exception& exception) {
    if (error != nullptr) {
      *error = QString::fromUtf8(exception.what());
    }
    return false;
  }
}

bool exportSvg(SheetScene& scene, const QString& path, bool transparent, QString* error) {
  if (!validatePage(scene, error)) {
    return false;
  }
  try {
    ExportSession session(scene, transparent);
    const QRectF page = scene.pageRect();
    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setTitle(QStringLiteral("FioVivo diagram"));
    generator.setDescription(scene.sheetName());
    generator.setSize(
        QSize(static_cast<int>(std::ceil(page.width() * kPixelsPerMillimetreAt96Dpi)),
              static_cast<int>(std::ceil(page.height() * kPixelsPerMillimetreAt96Dpi))));
    generator.setViewBox(QRectF(0.0, 0.0, page.width(), page.height()));
    generator.setResolution(96);
    QPainter painter;
    if (!painter.begin(&generator)) {
      if (error != nullptr) {
        *error = QObject::tr("Could not write the SVG file.");
      }
      return false;
    }
    scene.render(&painter, QRectF(0.0, 0.0, page.width(), page.height()), page);
    painter.end();
    return true;
  } catch (const std::exception& exception) {
    if (error != nullptr) {
      *error = QString::fromUtf8(exception.what());
    }
    return false;
  }
}

bool exportPdf(SheetScene& scene, const QString& path, QString* error) {
  if (!validatePage(scene, error)) {
    return false;
  }
  try {
    ExportSession session(scene, false);
    const QRectF page = scene.pageRect();
    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QSizeF(page.width(), page.height()), QPageSize::Millimeter));
    writer.setPageMargins(QMarginsF(0.0, 0.0, 0.0, 0.0), QPageLayout::Millimeter);
    writer.setResolution(300);
    writer.setTitle(QStringLiteral("FioVivo diagram"));
    writer.setCreator(QStringLiteral("FioVivo"));
    QPainter painter;
    if (!painter.begin(&writer)) {
      if (error != nullptr) {
        *error = QObject::tr("Could not write the PDF file.");
      }
      return false;
    }
    const QRectF target(0.0, 0.0, writer.width(), writer.height());
    scene.render(&painter, target, page);
    painter.end();
    return true;
  } catch (const std::exception& exception) {
    if (error != nullptr) {
      *error = QString::fromUtf8(exception.what());
    }
    return false;
  }
}

bool printScene(SheetScene& scene, QPrinter& printer, QString* error) {
  if (!validatePage(scene, error)) {
    return false;
  }
  try {
    ExportSession session(scene, false);
    const QRectF page = scene.pageRect();
    printer.setPageSize(QPageSize(QSizeF(page.width(), page.height()), QPageSize::Millimeter));
    printer.setPageMargins(QMarginsF(0.0, 0.0, 0.0, 0.0), QPageLayout::Millimeter);
    QPainter painter;
    if (!painter.begin(&printer)) {
      if (error != nullptr) {
        *error = QObject::tr("Could not start printing.");
      }
      return false;
    }
    const QRectF target(0.0, 0.0, printer.width(), printer.height());
    scene.render(&painter, target, page);
    painter.end();
    return true;
  } catch (const std::exception& exception) {
    if (error != nullptr) {
      *error = QString::fromUtf8(exception.what());
    }
    return false;
  }
}

} // namespace fiovivo::ui
