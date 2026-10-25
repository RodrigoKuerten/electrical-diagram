#include "ui/component_item.hpp"

#include "core/model.hpp"
#include "core/types.hpp"
#include "library/svg_renderer.hpp"
#include "ui/document.hpp"
#include "ui/pin_item.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/ui_helpers.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QFontMetricsF>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QToolTip>
#include <QtAlgorithms>

#include <algorithm>
#include <cmath>

namespace fiovivo::ui {
namespace {

[[nodiscard]] bool startsWith(std::string_view value, std::string_view prefix) {
  return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] bool isSelectorType(std::string_view type) {
  return type == component_type::kSelectorSwitch || startsWith(type, "selector_");
}

[[nodiscard]] bool isProtection(std::string_view type) {
  return type == component_type::kFuse || type == component_type::kBreaker ||
         type == component_type::kThermalRelay || startsWith(type, "fuse") ||
         startsWith(type, "breaker") || startsWith(type, "rcd_") || type == "motor_breaker_3p";
}

[[nodiscard]] bool isSwitchInput(std::string_view type) {
  return startsWith(type, "switch_") || startsWith(type, "limit_") || startsWith(type, "sensor_") ||
         startsWith(type, "pressure_") || startsWith(type, "vacuum_") ||
         startsWith(type, "thermostat_") || startsWith(type, "float_") ||
         startsWith(type, "reed_") || startsWith(type, "mushroom_");
}

[[nodiscard]] QString
shortState(const Component& component, const SimItemState& state, bool has_state) {
  if (!has_state) {
    return {};
  }
  using namespace component_type;
  if (isContactorContact(component.type)) {
    return state.closed ? QCoreApplication::translate("ComponentItem", "CLOSED")
                        : QCoreApplication::translate("ComponentItem", "OPEN");
  }
  if (isPushbutton(component.type) || isSwitchInput(component.type) ||
      component.type == kEmergencyStop) {
    return state.input_active ? QCoreApplication::translate("ComponentItem", "ON") : QString{};
  }
  if (isCoil(component.type) || component.type == kPilotLamp ||
      startsWith(component.type, "lamp_") || startsWith(component.type, "motor_") ||
      component.type == kMotor3ph || component.type == "buzzer" || component.type == "siren" ||
      component.type == "bell" || component.type == "horn" || component.type == "led") {
    return state.energized ? QCoreApplication::translate("ComponentItem", "ON")
                           : QCoreApplication::translate("ComponentItem", "OFF");
  }
  if (component.type == kFuse && state.tripped) {
    return QCoreApplication::translate("ComponentItem", "BLOWN");
  }
  if (isProtection(component.type) && state.tripped) {
    return QCoreApplication::translate("ComponentItem", "TRIP");
  }
  if (isSelectorType(component.type)) {
    return QCoreApplication::translate("ComponentItem", "P%1").arg(state.selector_position);
  }
  if (startsWith(component.type, "logic_")) {
    return state.logic_output ? QCoreApplication::translate("ComponentItem", "ON") : QString{};
  }
  return {};
}

} // namespace

ComponentItem::ComponentItem(SheetScene* scene, ComponentId id) : scene_(scene), id_(id) {
  setFlag(ItemIsSelectable, true);
  setFlag(ItemIsMovable, true);
  setFlag(ItemSendsGeometryChanges, true);
  setAcceptHoverEvents(true);
  rebuildPins();
  refreshFromModel();
}

ComponentItem::~ComponentItem() = default;

const Component& ComponentItem::component() const {
  return *scene_->document()->diagram().findComponent(id_);
}

QRectF ComponentItem::boundingRect() const {
  const Component& item = component();
  return QRectF(-12.0, -16.0, item.size.x + 24.0, item.size.y + 31.0);
}

void ComponentItem::rebuildPins() {
  qDeleteAll(pins_);
  pins_.clear();
  const Component& item = component();
  for (const Pin& pin : item.pins) {
    auto* pin_item = new PinItem(this,
                                 pin.id,
                                 QString::fromStdString(pin.name),
                                 pin.role,
                                 QPointF(pin.offset.x, pin.offset.y),
                                 static_cast<int>(pin.wires.size()));
    pins_.push_back(pin_item);
  }
}

void ComponentItem::refreshFromModel() {
  const Component& item = component();
  setPos(item.position.x, item.position.y);
  setTransformOriginPoint(item.size.x / 2.0, item.size.y / 2.0);
  setRotation(static_cast<double>(rotationDegrees(item.rotation)));
  setZValue(static_cast<double>(parameterInt(item.parameters, "z_order", 0)));
  updateTooltip();
  update();
}

void ComponentItem::updateTooltip() {
  const Component& item = component();
  QString text = QString::fromStdString(item.tag);
  if (!item.description.empty()) {
    text += "\n" + QString::fromStdString(item.description);
  }
  if (has_sim_) {
    text += "\n" + componentStateText(item, sim_);
  }
  setToolTip(text);
}

QPointF ComponentItem::pinScenePosition(PinId pin) const {
  for (PinItem* item : pins_) {
    if (item->pinId() == pin) {
      return item->centerScenePosition();
    }
  }
  return mapToScene(QPointF(0.0, 0.0));
}

PinItem* ComponentItem::pinItem(PinId pin) const {
  for (PinItem* item : pins_) {
    if (item->pinId() == pin) {
      return item;
    }
  }
  return nullptr;
}

std::string ComponentItem::symbolState() const {
  if (!has_sim_) {
    return {};
  }
  return simulationStateKey(component(), sim_);
}

QString ComponentItem::statusText() const {
  const Component& item = component();
  if (!has_sim_) {
    return {};
  }
  QString status = shortState(item, sim_, true);
  if (item.type == component_type::kTimerOn || item.type == component_type::kTimerOff) {
    const std::int64_t delay = parameterInt(item.parameters, "delay_ms", 0);
    if (delay > 0) {
      const double seconds = static_cast<double>(sim_.timer_elapsed_ms) / 1000.0;
      status = QStringLiteral("%1 s").arg(QString::number(seconds, 'f', 1));
    }
  }
  return status;
}

void ComponentItem::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem* option,
                          QWidget* widget) {
  (void)option;
  (void)widget;
  const Component& item = component();
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  const QRectF symbol_rect(0.0, 0.0, item.size.x, item.size.y);
  if (has_sim_ && sim_.energized) {
    const QColor highlight = scene_->energizedColor();
    painter->setPen(QPen(highlight, 1.0));
    painter->setBrush(QColor(highlight.red(), highlight.green(), highlight.blue(), 28));
    painter->drawRoundedRect(symbol_rect.adjusted(-1.5, -1.5, 1.5, 1.5), 2.0, 2.0);
  }

  if (auto renderer = scene_->renderCache()->renderer(item.type)) {
    renderer->paint(*painter, symbol_rect, symbolState());
  } else {
    painter->setPen(QPen(scene_->themePalette().color(ThemeToken::StatusError), 0.8));
    painter->drawRect(symbol_rect);
    painter->drawText(symbol_rect, Qt::AlignCenter, QString::fromStdString(item.type));
  }

  // Scene units are millimetres. Qt fonts are sized in points, so a screen-sized font would
  // be many millimetres tall here and get clipped by its label rectangle. Use a small point
  // size that maps to roughly 3 mm of text on the sheet.
  QFont tag_font = QApplication::font();
  tag_font.setBold(true);
  tag_font.setPointSizeF(4.2);
  painter->setFont(tag_font);
  painter->setPen(scene_->textColor());
  QString tag = QString::fromStdString(item.tag);
  const QString status = statusText();
  if (!status.isEmpty()) {
    tag += QStringLiteral(" [%1]").arg(status);
  }
  const auto draw_label = [&](const QString& text, const QFont& font, const QRectF& area,
                              bool right_aligned) {
    const QFontMetricsF metrics(font);
    const QRectF text_bounds = metrics.boundingRect(text);
    const double x = right_aligned ? area.right() - text_bounds.right()
                                   : area.center().x() - text_bounds.center().x();
    const QPointF origin(x, area.center().y() - text_bounds.center().y());
    QPainterPath path;
    path.addText(origin, font, text);
    // Draw a thin white outline instead of an opaque label background. Wires remain visible
    // around the glyphs, while the tag stays readable when a wire passes behind it.
    const QColor outline = scene_->themePalette().color(ThemeToken::SheetBackground);
    painter->setPen(QPen(outline, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(outline);
    painter->drawPath(path);
    painter->setPen(Qt::NoPen);
    painter->setBrush(scene_->textColor());
    painter->drawPath(path);
  };
  if (item.labels.show_tag) {
    // CADe_SIMU prints the reference immediately to the left of the symbol, vertically
    // centred.  The old implementation always centred it above the symbol, which pushed
    // references into neighbouring devices on dense CADe-size drawings.
    draw_label(tag, tag_font, QRectF(-42.0, item.size.y / 2.0 - 4.0, 40.0, 8.0), true);
  }
  if (item.labels.show_description && !item.description.empty()) {
    QFont description_font = QApplication::font();
    description_font.setPointSizeF(3.5);
    draw_label(QString::fromStdString(item.description), description_font,
               QRectF(-45.0, item.size.y + 3.0, item.size.x + 90.0, 7.0), false);
  }

  if (isSelected()) {
    painter->setPen(QPen(scene_->selectionColor(), 0.8, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(QRectF(-2.0, -2.0, item.size.x + 4.0, item.size.y + 4.0));
  }
  painter->restore();
}

void ComponentItem::setSimulationState(const SimItemState& state) {
  sim_ = state;
  has_sim_ = true;
  simulation_enabled_ = true;
  setFlag(ItemIsMovable, false);
  updateTooltip();
  update();
}

void ComponentItem::clearSimulationState() {
  has_sim_ = false;
  sim_ = SimItemState{};
  simulation_enabled_ = false;
  setFlag(ItemIsMovable, true);
  updateTooltip();
  update();
}

void ComponentItem::setSimulationEnabled(bool enabled) {
  simulation_enabled_ = enabled;
  setFlag(ItemIsMovable, !enabled);
  if (!enabled) {
    has_sim_ = false;
    sim_ = SimItemState{};
  }
  update();
}

void ComponentItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  if (event->button() != Qt::LeftButton) {
    QGraphicsItem::mousePressEvent(event);
    return;
  }
  if (simulation_enabled_) {
    const Component& item = component();
    const SimItemState state = scene_->simulationState(id_).value_or(SimItemState{});
    if (isPushbutton(item.type) || isSwitchInput(item.type)) {
      if (parameterString(item.parameters, "action", "") == "reset_emergency") {
        scene_->requestEmergencyReset();
        setSelected(true);
        event->accept();
        return;
      }
      const bool maintained = parameterString(item.parameters, "mode", "momentary") == "maintained";
      const bool next = maintained ? !state.input_active : true;
      pressed_for_simulation_ = true;
      scene_->requestInput(id_.value(), next);
      setSelected(true);
      event->accept();
      return;
    }
    if (item.type == component_type::kEmergencyStop) {
      scene_->requestInput(id_.value(), true);
      setSelected(true);
      event->accept();
      return;
    }
    if (item.type == component_type::kSelectorSwitch) {
      const int positions =
          std::max(1, static_cast<int>(parameterInt(item.parameters, "positions", 2)));
      int next = state.selector_position + 1;
      if (next > positions) {
        next = 1;
      }
      scene_->requestSelector(id_.value(), next);
      setSelected(true);
      event->accept();
      return;
    }
    if (isProtection(item.type)) {
      setSelected(true);
      event->accept();
      return;
    }
    QGraphicsItem::mousePressEvent(event);
    return;
  }
  QGraphicsItem::mousePressEvent(event);
}

void ComponentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  if (simulation_enabled_ && pressed_for_simulation_) {
    pressed_for_simulation_ = false;
    const Component& item = component();
    if ((isPushbutton(item.type) || isSwitchInput(item.type)) &&
        parameterString(item.parameters, "mode", "momentary") != "maintained") {
      scene_->requestInput(id_.value(), false);
    }
    event->accept();
    return;
  }
  QGraphicsItem::mouseReleaseEvent(event);
}

void ComponentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
  if (simulation_enabled_) {
    const Component& item = component();
    if (isProtection(item.type)) {
      const SimItemState state = scene_->simulationState(id_).value_or(SimItemState{});
      scene_->requestTripped(id_.value(), !state.tripped);
      event->accept();
      return;
    }
  } else {
    scene_->requestProperties(id_);
    event->accept();
    return;
  }
  QGraphicsItem::mouseDoubleClickEvent(event);
}

void ComponentItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  scene_->showComponentContextMenu(this, event->screenPos());
  event->accept();
}

} // namespace fiovivo::ui
