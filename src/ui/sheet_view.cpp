#include "ui/sheet_view.hpp"

#include "ui/component_item.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/wire_item.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace fiovivo::ui {
namespace {
constexpr double kBaseScale = 3.7795275591; // 96 dpi, one scene unit = 1 mm
constexpr double kMinZoom = 0.25;
constexpr double kMaxZoom = 4.0;
} // namespace

SheetView::SheetView(QWidget* parent) : QGraphicsView(parent) {
  setRenderHint(QPainter::Antialiasing, true);
  setRenderHint(QPainter::TextAntialiasing, true);
  setDragMode(QGraphicsView::RubberBandDrag);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);
  setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  setBackgroundBrush(Qt::NoBrush);
  setFrameShape(QFrame::NoFrame);
  applyAbsoluteZoom(1.0);
}

void SheetView::setSheetScene(SheetScene* scene) {
  sheet_scene_ = scene;
  setScene(scene);
  if (scene != nullptr) {
    setSceneRect(scene->sceneRect());
    fitToPage();
  }
}

int SheetView::zoomPercent() const { return static_cast<int>(std::lround(zoom_ * 100.0)); }

void SheetView::applyAbsoluteZoom(double zoom) {
  zoom_ = std::clamp(zoom, kMinZoom, kMaxZoom);
  const double factor = kBaseScale * zoom_;
  setTransform(QTransform::fromScale(factor, factor));
  emit zoomChanged(zoomPercent());
}

void SheetView::setZoomPercent(int percent) {
  const QPointF center = mapToScene(viewport()->rect().center());
  applyAbsoluteZoom(static_cast<double>(percent) / 100.0);
  centerOn(center);
}

void SheetView::fitToPage() {
  if (sheet_scene_ == nullptr) {
    return;
  }
  const QRectF target = sheet_scene_->pageRect().adjusted(-5.0, -5.0, 5.0, 5.0);
  if (target.isEmpty()) {
    return;
  }
  setTransformationAnchor(QGraphicsView::NoAnchor);
  resetTransform();
  fitInView(target, Qt::KeepAspectRatio);
  const double fitted = transform().m11() / kBaseScale;
  zoom_ = std::clamp(fitted, kMinZoom, kMaxZoom);
  if (std::abs(fitted - zoom_) > 0.001) {
    setTransform(QTransform::fromScale(kBaseScale * zoom_, kBaseScale * zoom_));
  }
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  centerOn(target.center());
  emit zoomChanged(zoomPercent());
}

void SheetView::zoomIn() {
  const QPointF center = mapToScene(viewport()->rect().center());
  setTransformationAnchor(QGraphicsView::NoAnchor);
  applyAbsoluteZoom(zoom_ * 1.25);
  centerOn(center);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void SheetView::zoomOut() {
  const QPointF center = mapToScene(viewport()->rect().center());
  setTransformationAnchor(QGraphicsView::NoAnchor);
  applyAbsoluteZoom(zoom_ / 1.25);
  centerOn(center);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void SheetView::centerOnComponent(ComponentId id) {
  if (sheet_scene_ == nullptr) {
    return;
  }
  if (auto* item = sheet_scene_->componentItem(id)) {
    centerOn(item->mapToScene(item->boundingRect().center()));
  }
}

void SheetView::centerOnWire(WireId id) {
  if (sheet_scene_ == nullptr) {
    return;
  }
  if (auto* item = sheet_scene_->wireItem(id)) {
    centerOn(item->boundingRect().center());
  }
}

void SheetView::wheelEvent(QWheelEvent* event) {
  if (event->angleDelta().y() == 0) {
    return;
  }
  const double factor = std::pow(1.0015, static_cast<double>(event->angleDelta().y()));
  const double target = std::clamp(zoom_ * factor, kMinZoom, kMaxZoom);
  const double applied = target / zoom_;
  if (std::abs(applied - 1.0) > 0.0001) {
    zoom_ = target;
    scale(applied, applied);
    emit zoomChanged(zoomPercent());
  }
  event->accept();
}

void SheetView::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton ||
      (event->button() == Qt::LeftButton && space_pressed_)) {
    panning_ = true;
    pan_origin_ = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }
  QGraphicsView::mousePressEvent(event);
}

void SheetView::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    const QPoint delta = event->pos() - pan_origin_;
    pan_origin_ = event->pos();
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    event->accept();
    return;
  }
  QGraphicsView::mouseMoveEvent(event);
}

void SheetView::mouseReleaseEvent(QMouseEvent* event) {
  if (panning_ && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
    panning_ = false;
    updateCursor();
    event->accept();
    return;
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void SheetView::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
    space_pressed_ = true;
    updateCursor();
    event->accept();
    return;
  }
  if (sheet_scene_ == nullptr) {
    QGraphicsView::keyPressEvent(event);
    return;
  }
  if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
    sheet_scene_->deleteSelection();
    event->accept();
    return;
  }
  if (event->key() == Qt::Key_R && !(event->modifiers() & Qt::ControlModifier)) {
    sheet_scene_->rotateSelection();
    event->accept();
    return;
  }
  if (event->key() == Qt::Key_W) {
    sheet_scene_->setWireMode(!sheet_scene_->wireMode());
    event->accept();
    return;
  }
  if (event->key() == Qt::Key_Escape) {
    sheet_scene_->cancelWiring();
    event->accept();
    return;
  }
  const double step = sheet_scene_->gridSize();
  QPointF delta;
  if (event->key() == Qt::Key_Left) {
    delta = {-step, 0.0};
  } else if (event->key() == Qt::Key_Right) {
    delta = {step, 0.0};
  } else if (event->key() == Qt::Key_Up) {
    delta = {0.0, -step};
  } else if (event->key() == Qt::Key_Down) {
    delta = {0.0, step};
  }
  if (!delta.isNull() && !sheet_scene_->selectedComponentIds().isEmpty()) {
    sheet_scene_->moveSelection(delta);
    event->accept();
    return;
  }
  QGraphicsView::keyPressEvent(event);
}

void SheetView::keyReleaseEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
    space_pressed_ = false;
    updateCursor();
    event->accept();
    return;
  }
  QGraphicsView::keyReleaseEvent(event);
}

void SheetView::updateCursor() {
  if (space_pressed_ && !panning_) {
    setCursor(Qt::OpenHandCursor);
  } else if (panning_) {
    setCursor(Qt::ClosedHandCursor);
  } else {
    unsetCursor();
  }
}

} // namespace fiovivo::ui
