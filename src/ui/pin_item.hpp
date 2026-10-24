#pragma once

#include "core/ids.hpp"
#include "core/types.hpp"

#include <QGraphicsItem>
#include <QString>

namespace fiovivo::ui {

class ComponentItem;

class PinItem : public QGraphicsItem {
public:
  PinItem(ComponentItem* component,
          PinId id,
          QString name,
          PinRole role,
          QPointF offset,
          int wire_count);

  [[nodiscard]] QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  [[nodiscard]] PinId pinId() const { return id_; }
  [[nodiscard]] QString pinName() const { return name_; }
  [[nodiscard]] PinRole role() const { return role_; }
  [[nodiscard]] ComponentItem* componentItem() const { return component_; }
  [[nodiscard]] QPointF centerScenePosition() const;
  void setWireCount(int count);
  void setHighlighted(bool highlighted);

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
  void updateTooltip();

  ComponentItem* component_ = nullptr;
  PinId id_;
  QString name_;
  PinRole role_ = PinRole::Terminal;
  int wire_count_ = 0;
  bool highlighted_ = false;
};

} // namespace fiovivo::ui
