#pragma once

#include "core/ids.hpp"
#include "core/types.hpp"

#include <QGraphicsItem>

namespace fiovivo::ui {

class SheetScene;

class AnnotationItem : public QGraphicsItem {
public:
  static constexpr int Type = QGraphicsItem::UserType + 3;

  AnnotationItem(SheetScene* scene, AnnotationId id);

  [[nodiscard]] int type() const override { return Type; }
  [[nodiscard]] QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  [[nodiscard]] AnnotationId annotationId() const { return id_; }
  void refreshFromModel();

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
  SheetScene* scene_ = nullptr;
  AnnotationId id_;
  AnnotationKind kind_ = AnnotationKind::Text;
  QRectF rect_;
  QString text_;
  double font_size_ = 3.5;
  QColor color_;
};

} // namespace fiovivo::ui
