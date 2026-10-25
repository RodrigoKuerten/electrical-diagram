#pragma once

#include "core/ids.hpp"
#include "core/model.hpp"
#include "ui/sim_ui_types.hpp"
#include "ui/theme.hpp"

#include <QColor>
#include <QGraphicsScene>
#include <QHash>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QSet>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class QGraphicsPathItem;

namespace fiovivo {
class SymbolLibrary;
class SymbolRenderCache;
} // namespace fiovivo

namespace fiovivo::ui {

class AnnotationItem;
class ComponentItem;
class Document;
class PinItem;
class WireItem;

class SheetScene : public QGraphicsScene {
  Q_OBJECT

public:
  SheetScene(Document* document,
             const SymbolLibrary* library,
             SymbolRenderCache* render_cache,
             QObject* parent = nullptr);
  ~SheetScene() override;

  [[nodiscard]] Document* document() const { return document_; }
  [[nodiscard]] const SymbolLibrary* symbolLibrary() const { return library_; }
  [[nodiscard]] SymbolRenderCache* renderCache() const { return render_cache_; }
  [[nodiscard]] SheetId sheetId() const { return sheet_id_; }
  [[nodiscard]] QRectF pageRect() const;
  // Usable drawing area. It is never smaller than the printed page, but may be larger.
  [[nodiscard]] QRectF workspaceRect() const;
  [[nodiscard]] QString sheetName() const;

  void setSheet(SheetId id);
  void rebuild();

  void setGridVisible(bool visible);
  [[nodiscard]] bool gridVisible() const { return grid_visible_; }
  void setGridSize(double size);
  [[nodiscard]] double gridSize() const { return grid_size_; }
  [[nodiscard]] QPointF snap(QPointF point) const;

  void setThemePalette(const ThemePalette& palette) {
    theme_ = palette;
    update();
  }
  [[nodiscard]] const ThemePalette& themePalette() const { return theme_; }
  [[nodiscard]] QColor wireColor() const { return theme_.color(ThemeToken::WireDefault); }
  [[nodiscard]] QColor energizedColor() const { return theme_.color(ThemeToken::WireEnergized); }
  [[nodiscard]] QColor selectionColor() const { return theme_.color(ThemeToken::Selection); }
  [[nodiscard]] QColor pinColor() const { return theme_.color(ThemeToken::SymbolStroke); }
  [[nodiscard]] QColor textColor() const { return theme_.color(ThemeToken::TextPrimary); }

  void setWireMode(bool enabled);
  [[nodiscard]] bool wireMode() const { return wire_mode_; }
  [[nodiscard]] bool isWiring() const { return wire_start_.has_value(); }
  void cancelWiring();

  void setSimulationInteractive(bool interactive);
  [[nodiscard]] bool simulationInteractive() const { return simulation_interactive_; }
  void setSimulationFrame(const SimFrame& frame);
  void clearSimulation();
  [[nodiscard]] std::optional<SimItemState> simulationState(ComponentId id) const;

  [[nodiscard]] ComponentItem* componentItem(ComponentId id) const;
  [[nodiscard]] WireItem* wireItem(WireId id) const;

  void addSymbolAt(const std::string& type, QPointF scene_position);
  void deleteSelection();
  void rotateSelection();
  void duplicateSelection();
  void copySelection();
  void pasteClipboard();
  void selectAllContent();
  void moveSelection(QPointF delta);
  [[nodiscard]] bool hasClipboard() const { return !clipboard_.empty(); }

  [[nodiscard]] QList<ComponentId> selectedComponentIds() const;
  [[nodiscard]] QList<WireId> selectedWireIds() const;
  [[nodiscard]] QList<AnnotationId> selectedAnnotationIds() const;
  void selectComponent(ComponentId id);
  void selectWire(WireId id);

  void setExportMode(bool enabled);
  void setExportBackgroundTransparent(bool transparent);
  [[nodiscard]] bool exportMode() const { return export_mode_; }

  void showComponentContextMenu(ComponentItem* item, QPoint screen_position);
  void showWireContextMenu(WireItem* item, QPoint screen_position);
  void showAnnotationContextMenu(AnnotationItem* item, QPoint screen_position);

  void requestInput(quint64 component, bool active);
  void requestTripped(quint64 component, bool tripped);
  void requestSelector(quint64 component, int position);
  void requestEmergencyReset();
  void requestProperties(ComponentId id);

signals:
  void cursorMoved(const QPointF& scene_position);
  void inputRequested(quint64 component, bool active);
  void tripRequested(quint64 component, bool tripped);
  void selectorRequested(quint64 component, int position);
  void emergencyResetRequested();
  void propertiesRequested(quint64 component);
  void wireModeChanged(bool enabled);
  void statusMessage(const QString& message);

protected:
  void drawBackground(QPainter* painter, const QRectF& rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
  void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
  void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
  void dropEvent(QGraphicsSceneDragDropEvent* event) override;

private:
  [[nodiscard]] const fiovivo::Sheet* sheet() const;
  [[nodiscard]] PinItem* pinAt(QPointF scene_position) const;
  [[nodiscard]] PinRef pinRefFor(const PinItem* pin) const;
  void beginWiring(PinRef start);
  void finishWiring(PinRef target);
  void updateWirePreview(QPointF cursor);
  void applySimulationToItems();
  void commitMoves();
  void bringSelectionToFront(bool front);
  void addAnnotation(AnnotationKind kind);

  Document* document_ = nullptr;
  const SymbolLibrary* library_ = nullptr;
  SymbolRenderCache* render_cache_ = nullptr;
  SheetId sheet_id_;

  QHash<quint64, ComponentItem*> component_items_;
  QHash<quint64, WireItem*> wire_items_;
  QHash<quint64, AnnotationItem*> annotation_items_;

  bool grid_visible_ = true;
  double grid_size_ = 5.0;
  bool wire_mode_ = false;
  bool simulation_interactive_ = false;
  bool export_mode_ = false;
  bool export_transparent_ = false;

  ThemePalette theme_ = lightTheme();

  std::optional<PinRef> wire_start_;
  bool wire_dragging_ = false;
  std::unique_ptr<QGraphicsPathItem> wire_preview_;

  std::vector<Component> clipboard_;
  int paste_count_ = 0;

  SimFrame simulation_frame_;
  bool has_simulation_frame_ = false;
};

} // namespace fiovivo::ui
