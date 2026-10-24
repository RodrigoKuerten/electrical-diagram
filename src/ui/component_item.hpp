#pragma once

#include "core/ids.hpp"
#include "core/model.hpp"
#include "ui/sim_ui_types.hpp"

#include <QGraphicsItem>
#include <QPointF>
#include <QString>

#include <memory>
#include <string>
#include <vector>

namespace fiovivo::ui {

class PinItem;
class SheetScene;

class ComponentItem : public QGraphicsItem {
public:
  static constexpr int Type = QGraphicsItem::UserType + 1;

  ComponentItem(SheetScene* scene, ComponentId id);
  ~ComponentItem() override;

  [[nodiscard]] int type() const override { return Type; }
  [[nodiscard]] QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  [[nodiscard]] ComponentId componentId() const { return id_; }
  [[nodiscard]] SheetScene* sheetScene() const { return scene_; }
  [[nodiscard]] const fiovivo::Component& component() const;
  [[nodiscard]] QPointF pinScenePosition(PinId pin) const;
  [[nodiscard]] PinItem* pinItem(PinId pin) const;
  [[nodiscard]] std::string symbolState() const;

  void refreshFromModel();
  void setSimulationState(const SimItemState& state);
  void clearSimulationState();
  void setSimulationEnabled(bool enabled);
  [[nodiscard]] bool hasSimulationState() const { return has_sim_; }
  [[nodiscard]] SimItemState simulationState() const { return sim_; }
  [[nodiscard]] QString statusText() const;

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
  void rebuildPins();
  void updateTooltip();

  SheetScene* scene_ = nullptr;
  ComponentId id_;
  std::vector<PinItem*> pins_;
  SimItemState sim_;
  bool has_sim_ = false;
  bool simulation_enabled_ = false;
  bool pressed_for_simulation_ = false;
};

} // namespace fiovivo::ui
