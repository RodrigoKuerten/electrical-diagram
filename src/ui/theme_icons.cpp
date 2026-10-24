#include "ui/theme_icons.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>

#include <array>

namespace fiovivo::ui {
namespace {

void drawIcon(QPainter& painter, ThemeIcon icon, const QColor& color) {
  QPen pen(color);
  pen.setWidthF(2.0);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  switch (icon) {
  case ThemeIcon::New: {
    QPainterPath page;
    page.moveTo(9.0, 5.0);
    page.lineTo(20.0, 5.0);
    page.lineTo(25.0, 10.0);
    page.lineTo(25.0, 27.0);
    page.lineTo(9.0, 27.0);
    page.closeSubpath();
    painter.drawPath(page);
    painter.drawLine(QPointF(20.0, 5.0), QPointF(20.0, 10.0));
    painter.drawLine(QPointF(20.0, 10.0), QPointF(25.0, 10.0));
    break;
  }
  case ThemeIcon::Open: {
    QPainterPath folder;
    folder.moveTo(5.0, 11.0);
    folder.lineTo(12.0, 11.0);
    folder.lineTo(14.0, 14.0);
    folder.lineTo(27.0, 14.0);
    folder.lineTo(24.0, 26.0);
    folder.lineTo(7.0, 26.0);
    folder.closeSubpath();
    painter.drawPath(folder);
    painter.drawLine(QPointF(5.0, 11.0), QPointF(5.0, 8.0));
    painter.drawLine(QPointF(5.0, 8.0), QPointF(13.0, 8.0));
    painter.drawLine(QPointF(13.0, 8.0), QPointF(15.0, 11.0));
    break;
  }
  case ThemeIcon::Save: {
    painter.drawRoundedRect(QRectF(7.0, 6.0, 20.0, 21.0), 2.0, 2.0);
    painter.drawRect(QRectF(12.0, 6.0, 10.0, 7.0));
    painter.drawRect(QRectF(11.0, 18.0, 12.0, 9.0));
    break;
  }
  case ThemeIcon::Undo:
  case ThemeIcon::Redo: {
    const bool redo = icon == ThemeIcon::Redo;
    painter.save();
    if (redo) {
      painter.translate(32.0, 0.0);
      painter.scale(-1.0, 1.0);
    }
    QPainterPath path;
    path.moveTo(10.0, 13.0);
    path.cubicTo(17.0, 8.0, 26.0, 10.0, 25.0, 19.0);
    path.cubicTo(24.0, 25.0, 18.0, 27.0, 13.0, 25.0);
    painter.drawPath(path);
    painter.drawLine(QPointF(10.0, 13.0), QPointF(15.0, 11.0));
    painter.drawLine(QPointF(10.0, 13.0), QPointF(11.0, 18.0));
    painter.restore();
    break;
  }
  case ThemeIcon::Cut: {
    painter.drawEllipse(QPointF(10.0, 23.0), 2.5, 2.5);
    painter.drawEllipse(QPointF(10.0, 9.0), 2.5, 2.5);
    painter.drawLine(QPointF(12.0, 11.0), QPointF(25.0, 24.0));
    painter.drawLine(QPointF(12.0, 21.0), QPointF(25.0, 8.0));
    break;
  }
  case ThemeIcon::Copy: {
    painter.drawRoundedRect(QRectF(6.0, 9.0, 15.0, 18.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(12.0, 5.0, 15.0, 18.0), 2.0, 2.0);
    break;
  }
  case ThemeIcon::Paste: {
    painter.drawRoundedRect(QRectF(8.0, 8.0, 17.0, 19.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(12.0, 5.0, 9.0, 6.0), 1.5, 1.5);
    painter.drawLine(QPointF(12.0, 15.0), QPointF(21.0, 15.0));
    painter.drawLine(QPointF(12.0, 20.0), QPointF(21.0, 20.0));
    break;
  }
  case ThemeIcon::ZoomIn:
  case ThemeIcon::ZoomOut: {
    painter.drawEllipse(QPointF(14.0, 14.0), 7.5, 7.5);
    painter.drawLine(QPointF(19.5, 19.5), QPointF(27.0, 27.0));
    painter.drawLine(QPointF(10.0, 14.0), QPointF(18.0, 14.0));
    if (icon == ThemeIcon::ZoomIn) {
      painter.drawLine(QPointF(14.0, 10.0), QPointF(14.0, 18.0));
    }
    break;
  }
  case ThemeIcon::Fit: {
    painter.drawRect(QRectF(8.0, 9.0, 16.0, 14.0));
    painter.drawLine(QPointF(4.0, 10.0), QPointF(4.0, 5.0));
    painter.drawLine(QPointF(4.0, 5.0), QPointF(9.0, 5.0));
    painter.drawLine(QPointF(28.0, 10.0), QPointF(28.0, 5.0));
    painter.drawLine(QPointF(28.0, 5.0), QPointF(23.0, 5.0));
    painter.drawLine(QPointF(4.0, 22.0), QPointF(4.0, 27.0));
    painter.drawLine(QPointF(4.0, 27.0), QPointF(9.0, 27.0));
    painter.drawLine(QPointF(28.0, 22.0), QPointF(28.0, 27.0));
    painter.drawLine(QPointF(28.0, 27.0), QPointF(23.0, 27.0));
    break;
  }
  case ThemeIcon::Play: {
    QPainterPath triangle;
    triangle.moveTo(11.0, 7.0);
    triangle.lineTo(25.0, 16.0);
    triangle.lineTo(11.0, 25.0);
    triangle.closeSubpath();
    painter.setBrush(color);
    painter.drawPath(triangle);
    break;
  }
  case ThemeIcon::Pause: {
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(9.0, 7.0, 5.0, 18.0), 1.0, 1.0);
    painter.drawRoundedRect(QRectF(18.0, 7.0, 5.0, 18.0), 1.0, 1.0);
    break;
  }
  case ThemeIcon::Step: {
    QPainterPath triangle;
    triangle.moveTo(9.0, 8.0);
    triangle.lineTo(21.0, 16.0);
    triangle.lineTo(9.0, 24.0);
    triangle.closeSubpath();
    painter.setBrush(color);
    painter.drawPath(triangle);
    painter.drawLine(QPointF(24.0, 8.0), QPointF(24.0, 24.0));
    break;
  }
  case ThemeIcon::Stop: {
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(8.0, 8.0, 16.0, 16.0), 2.0, 2.0);
    break;
  }
  case ThemeIcon::Grid: {
    painter.drawRect(QRectF(6.0, 6.0, 20.0, 20.0));
    for (int index = 1; index < 3; ++index) {
      const double offset = static_cast<double>(index) * 20.0 / 3.0;
      painter.drawLine(QPointF(6.0 + offset, 6.0), QPointF(6.0 + offset, 26.0));
      painter.drawLine(QPointF(6.0, 6.0 + offset), QPointF(26.0, 6.0 + offset));
    }
    break;
  }
  case ThemeIcon::Wire: {
    QPainterPath wire;
    wire.moveTo(5.0, 10.0);
    wire.lineTo(13.0, 10.0);
    wire.lineTo(13.0, 22.0);
    wire.lineTo(27.0, 22.0);
    painter.drawPath(wire);
    painter.setBrush(color);
    painter.drawEllipse(QPointF(5.0, 10.0), 2.0, 2.0);
    painter.drawEllipse(QPointF(27.0, 22.0), 2.0, 2.0);
    break;
  }
  case ThemeIcon::Validate: {
    painter.drawEllipse(QPointF(16.0, 16.0), 11.0, 11.0);
    painter.drawLine(QPointF(10.0, 16.0), QPointF(14.0, 21.0));
    painter.drawLine(QPointF(14.0, 21.0), QPointF(23.0, 11.0));
    break;
  }
  }
}

[[nodiscard]] QPixmap renderIcon(ThemeIcon icon, const QColor& color, int size) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.scale(static_cast<double>(size) / 32.0, static_cast<double>(size) / 32.0);
  drawIcon(painter, icon, color);
  painter.end();
  return pixmap;
}

} // namespace

QIcon themeIcon(ThemeIcon icon, const ThemePalette& palette) {
  QIcon result;
  const QColor color = palette.color(ThemeToken::IconPrimary);
  for (const int size : std::array<int, 5>{16, 24, 32, 48, 64}) {
    result.addPixmap(renderIcon(icon, color, size));
  }
  return result;
}

} // namespace fiovivo::ui
