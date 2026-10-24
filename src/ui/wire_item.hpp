#pragma once

#include "core/ids.hpp"
#include "core/types.hpp"

#include <QGraphicsItem>
#include <QPointF>

#include <vector>

namespace fiovivo::ui {

class SheetScene;

class WireItem : public QGraphicsItem {
public:
  static constexpr int Type = QGraphicsItem::UserType + 2;

  WireItem(SheetScene* scene, WireId id);

  [[nodiscard]] int type() const override { return Type; }
  [[nodiscard]] QRectF boundingRect() const override;
  [[nodiscard]] QPainterPath shape() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  [[nodiscard]] WireId wireId() const { return id_; }
  void refreshFromModel();
  void setLive(bool live);
  [[nodiscard]] bool isLive() const { return live_; }

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
  [[nodiscard]] QPainterPath path() const;

  SheetScene* scene_ = nullptr;
  WireId id_;
  std::vector<QPointF> route_;
  WireStyle style_;
  bool live_ = false;
};

} // namespace fiovivo::ui
