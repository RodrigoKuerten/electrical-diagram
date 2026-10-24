#pragma once

#include "core/ids.hpp"

#include <QGraphicsView>
#include <QPoint>

namespace fiovivo::ui {

class SheetScene;

class SheetView : public QGraphicsView {
  Q_OBJECT

public:
  explicit SheetView(QWidget* parent = nullptr);

  void setSheetScene(SheetScene* scene);
  [[nodiscard]] SheetScene* sheetScene() const { return sheet_scene_; }
  [[nodiscard]] int zoomPercent() const;
  void setZoomPercent(int percent);
  void fitToPage();
  void zoomIn();
  void zoomOut();
  void centerOnComponent(ComponentId id);
  void centerOnWire(WireId id);

signals:
  void zoomChanged(int percent);

protected:
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

private:
  void applyAbsoluteZoom(double zoom);
  void updateCursor();

  SheetScene* sheet_scene_ = nullptr;
  double zoom_ = 1.0;
  bool space_pressed_ = false;
  bool panning_ = false;
  QPoint pan_origin_;
};

} // namespace fiovivo::ui
