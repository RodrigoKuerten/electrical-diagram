#include "ui/annotation_item.hpp"

#include "core/model.hpp"
#include "ui/document.hpp"
#include "ui/sheet_scene.hpp"

#include <QFont>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace fiovivo::ui {

AnnotationItem::AnnotationItem(SheetScene* scene, AnnotationId id) : scene_(scene), id_(id) {
  setFlag(ItemIsSelectable, true);
  setZValue(-2.0);
  refreshFromModel();
}

QRectF AnnotationItem::boundingRect() const { return rect_.adjusted(-2.0, -2.0, 2.0, 2.0); }

void AnnotationItem::refreshFromModel() {
  const Sheet* sheet = scene_->document()->currentSheet();
  if (sheet == nullptr) {
    return;
  }
  const Annotation* annotation = nullptr;
  for (const Annotation& candidate : sheet->annotations) {
    if (candidate.id == id_) {
      annotation = &candidate;
      break;
    }
  }
  if (annotation == nullptr) {
    return;
  }
  kind_ = annotation->kind;
  rect_ = QRectF(
      annotation->position.x, annotation->position.y, annotation->size.x, annotation->size.y);
  text_ = QString::fromStdString(annotation->text);
  font_size_ = annotation->font_size;
  color_ = QColor(annotation->color.c_str());
  prepareGeometryChange();
  update();
}

void AnnotationItem::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem* option,
                           QWidget* widget) {
  (void)option;
  (void)widget;
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  QColor color = color_.isValid() ? color_ : scene_->textColor();
  if (kind_ == AnnotationKind::Rectangle || kind_ == AnnotationKind::Legend) {
    painter->setPen(QPen(color, 0.6));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect_);
  }
  if (!text_.isEmpty()) {
    QFont font = painter->font();
    font.setPointSizeF(std::max(2.6, font_size_ * 0.95));
    painter->setFont(font);
    painter->setPen(color);
    const QRectF text_rect = rect_.adjusted(1.5, 1.0, -1.5, -1.0);
    painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text_);
  }
  if (isSelected()) {
    painter->setPen(QPen(scene_->selectionColor(), 0.6, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect_.adjusted(-1.0, -1.0, 1.0, 1.0));
  }
  painter->restore();
}

void AnnotationItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  scene_->showAnnotationContextMenu(this, event->screenPos());
  event->accept();
}

} // namespace fiovivo::ui
