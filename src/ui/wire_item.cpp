#include "ui/wire_item.hpp"

#include "core/model.hpp"
#include "ui/document.hpp"
#include "ui/sheet_scene.hpp"

#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>

namespace fiovivo::ui {

WireItem::WireItem(SheetScene* scene, WireId id) : scene_(scene), id_(id) {
  setFlag(ItemIsSelectable, true);
  setZValue(-1.0);
  refreshFromModel();
}

QRectF WireItem::boundingRect() const {
  if (route_.empty()) {
    return {};
  }
  QRectF bounds(route_.front(), route_.front());
  for (const QPointF& point : route_) {
    bounds = bounds.united(QRectF(point, QSizeF(0.0, 0.0)));
  }
  return bounds.adjusted(-3.0, -3.0, 3.0, 3.0);
}

QPainterPath WireItem::path() const {
  QPainterPath result;
  if (route_.empty()) {
    return result;
  }
  result.moveTo(route_.front());
  for (std::size_t index = 1; index < route_.size(); ++index) {
    result.lineTo(route_[index]);
  }
  return result;
}

QPainterPath WireItem::shape() const {
  QPainterPathStroker stroker;
  stroker.setWidth(2.0);
  return stroker.createStroke(path());
}

void WireItem::refreshFromModel() {
  const Wire* wire = scene_->document()->diagram().findWire(id_);
  if (wire == nullptr) {
    return;
  }
  route_.clear();
  for (const Vec2& point : wire->route) {
    route_.emplace_back(point.x, point.y);
  }
  style_ = wire->style;
  prepareGeometryChange();
  update();
}

void WireItem::setLive(bool live) {
  if (live_ == live) {
    return;
  }
  live_ = live;
  update();
}

void WireItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  (void)option;
  (void)widget;
  if (route_.empty()) {
    return;
  }
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QColor base(style_.color.c_str());
  if (!base.isValid()) {
    base = scene_->wireColor();
  }
  if (live_) {
    QColor glow = scene_->energizedColor();
    glow.setAlpha(90);
    QPen halo(glow, style_.thickness + 2.4);
    halo.setCapStyle(Qt::RoundCap);
    halo.setJoinStyle(Qt::RoundJoin);
    painter->setPen(halo);
    painter->drawPath(path());
    base = scene_->energizedColor();
  }
  QPen pen(base, style_.thickness);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  switch (style_.dash) {
  case WireDash::Solid:
    pen.setStyle(Qt::SolidLine);
    break;
  case WireDash::Dashed:
    pen.setStyle(Qt::DashLine);
    break;
  case WireDash::Dotted:
    pen.setStyle(Qt::DotLine);
    break;
  }
  painter->setPen(pen);
  painter->drawPath(path());
  if (isSelected()) {
    painter->setPen(QPen(scene_->selectionColor(), 0.7, Qt::DashLine));
    painter->drawPath(path());
  }
  painter->restore();
}

void WireItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  scene_->showWireContextMenu(this, event->screenPos());
  event->accept();
}

} // namespace fiovivo::ui
