#include "ui/pin_item.hpp"

#include "ui/component_item.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/ui_helpers.hpp"

#include <QCoreApplication>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

namespace fiovivo::ui {

PinItem::PinItem(
    ComponentItem* component, PinId id, QString name, PinRole role, QPointF offset, int wire_count)
    : QGraphicsItem(component), component_(component), id_(id), name_(std::move(name)), role_(role),
      wire_count_(wire_count) {
  setPos(offset);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::NoButton);
  setZValue(5.0);
  updateTooltip();
}

QRectF PinItem::boundingRect() const { return QRectF(-2.5, -2.5, 5.0, 5.0); }

void PinItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  (void)option;
  (void)widget;
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  const QColor base = component_ != nullptr && component_->sheetScene() != nullptr
                          ? component_->sheetScene()->pinColor()
                          : QColor("#1f2933");
  QColor color = base;
  if (highlighted_) {
    color = component_->sheetScene()->energizedColor();
    painter->setPen(QPen(color, 1.2));
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), 40));
    painter->drawEllipse(QPointF(0.0, 0.0), 2.0, 2.0);
  }
  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawEllipse(QPointF(0.0, 0.0), 0.65, 0.65);
  if (wire_count_ >= 3) {
    painter->drawEllipse(QPointF(0.0, 0.0), 1.5, 1.5);
  }
  painter->restore();
}

QPointF PinItem::centerScenePosition() const { return mapToScene(QPointF(0.0, 0.0)); }

void PinItem::setWireCount(int count) {
  if (wire_count_ == count) {
    return;
  }
  wire_count_ = count;
  updateTooltip();
  update();
}

void PinItem::setHighlighted(bool highlighted) {
  if (highlighted_ == highlighted) {
    return;
  }
  highlighted_ = highlighted;
  update();
}

void PinItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
  QGraphicsItem::hoverEnterEvent(event);
  setHighlighted(true);
  // Let QGraphicsScene show the item tooltip. Showing QToolTip manually here as well causes
  // flicker and can cover nearby wires/text with two competing tooltip windows.
}

void PinItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
  QGraphicsItem::hoverLeaveEvent(event);
  setHighlighted(false);
}

void PinItem::updateTooltip() {
  setToolTip(QStringLiteral("%1\n%2\n%3")
                 .arg(name_,
                      pinRoleText(role_),
                      QCoreApplication::translate(
                          "PinItem", "%n wire(s) connected", nullptr, wire_count_)));
}

} // namespace fiovivo::ui
