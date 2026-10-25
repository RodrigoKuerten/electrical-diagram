#include "ui/sheet_scene.hpp"

#include "core/geometry.hpp"
#include "core/model.hpp"
#include "core/types.hpp"
#include "library/svg_renderer.hpp"
#include "library/symbol_definition.hpp"
#include "ui/annotation_item.hpp"
#include "ui/component_factory.hpp"
#include "ui/component_item.hpp"
#include "ui/document.hpp"
#include "ui/pin_item.hpp"
#include "ui/ui_helpers.hpp"
#include "ui/wire_item.hpp"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QGraphicsPathItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTimer>
#include <QToolTip>

#include <algorithm>
#include <cmath>
#include <utility>

namespace fiovivo::ui {
namespace {

constexpr const char* kSymbolMimeType = "application/x-fiovivo-symbol";

} // namespace

SheetScene::SheetScene(Document* document,
                       const SymbolLibrary* library,
                       SymbolRenderCache* render_cache,
                       QObject* parent)
    : QGraphicsScene(parent), document_(document), library_(library), render_cache_(render_cache) {
  sheet_id_ = document_->currentSheetId();
  setBackgroundBrush(Qt::NoBrush);
  connect(document_, &Document::diagramChanged, this, &SheetScene::rebuild);
  connect(document_, &Document::currentSheetChanged, this, [this] {
    sheet_id_ = document_->currentSheetId();
    rebuild();
  });
  rebuild();
}

SheetScene::~SheetScene() = default;

const fiovivo::Sheet* SheetScene::sheet() const {
  return document_->diagram().findSheet(sheet_id_);
}

QRectF SheetScene::pageRect() const {
  const Sheet* current = sheet();
  if (current == nullptr) {
    return QRectF(0.0, 0.0, 297.0, 210.0);
  }
  const Vec2 size = paperDimensions(current->setup.paper, current->setup.orientation);
  return QRectF(0.0, 0.0, size.x, size.y);
}

QRectF SheetScene::workspaceRect() const {
  const QRectF page = pageRect();
  const Sheet* current = sheet();
  if (current == nullptr) {
    return page;
  }
  const Vec2 configured = current->setup.workspace_size;
  return QRectF(
      0.0, 0.0, std::max(page.width(), configured.x), std::max(page.height(), configured.y));
}

QString SheetScene::sheetName() const {
  const Sheet* current = sheet();
  return current == nullptr ? QString{} : QString::fromStdString(current->name);
}

void SheetScene::setSheet(SheetId id) {
  if (sheet_id_ == id && !component_items_.isEmpty()) {
    return;
  }
  sheet_id_ = id;
  rebuild();
}

void SheetScene::rebuild() {
  if (wire_preview_) {
    removeItem(wire_preview_.get());
    wire_preview_.reset();
  }
  wire_start_.reset();
  wire_dragging_ = false;

  QSet<quint64> selected_components;
  QSet<quint64> selected_wires;
  QSet<quint64> selected_annotations;
  for (ComponentItem* item : std::as_const(component_items_)) {
    if (item->isSelected()) {
      selected_components.insert(item->componentId().value());
    }
  }
  for (WireItem* item : std::as_const(wire_items_)) {
    if (item->isSelected()) {
      selected_wires.insert(item->wireId().value());
    }
  }
  for (AnnotationItem* item : std::as_const(annotation_items_)) {
    if (item->isSelected()) {
      selected_annotations.insert(item->annotationId().value());
    }
  }

  QGraphicsScene::clear();
  component_items_.clear();
  wire_items_.clear();
  annotation_items_.clear();

  const Sheet* current = sheet();
  if (current == nullptr) {
    setSceneRect(QRectF(-20.0, -20.0, 340.0, 260.0));
    update();
    return;
  }
  sheet_id_ = current->id;

  for (const Wire& wire : current->wires) {
    auto item = std::make_unique<WireItem>(this, wire.id);
    WireItem* raw = item.get();
    addItem(item.release());
    wire_items_.insert(wire.id.value(), raw);
  }
  for (const Component& component : current->components) {
    auto item = std::make_unique<ComponentItem>(this, component.id);
    ComponentItem* raw = item.get();
    addItem(item.release());
    component_items_.insert(component.id.value(), raw);
  }
  for (const Annotation& annotation : current->annotations) {
    auto item = std::make_unique<AnnotationItem>(this, annotation.id);
    AnnotationItem* raw = item.get();
    addItem(item.release());
    annotation_items_.insert(annotation.id.value(), raw);
  }

  for (auto [id, item] : component_items_.asKeyValueRange()) {
    if (selected_components.contains(id)) {
      item->setSelected(true);
    }
  }
  for (auto [id, item] : wire_items_.asKeyValueRange()) {
    if (selected_wires.contains(id)) {
      item->setSelected(true);
    }
  }
  for (auto [id, item] : annotation_items_.asKeyValueRange()) {
    if (selected_annotations.contains(id)) {
      item->setSelected(true);
    }
  }

  setSceneRect(workspaceRect().adjusted(-20.0, -20.0, 20.0, 20.0));
  for (ComponentItem* item : std::as_const(component_items_)) {
    item->setSimulationEnabled(simulation_interactive_);
  }
  if (has_simulation_frame_) {
    applySimulationToItems();
  }
  update();
}

void SheetScene::setGridVisible(bool visible) {
  grid_visible_ = visible;
  update();
}

void SheetScene::setGridSize(double size) {
  grid_size_ = std::max(1.0, size);
  update();
}

QPointF SheetScene::snap(QPointF point) const {
  if (grid_size_ <= 0.0) {
    return point;
  }
  return QPointF(std::round(point.x() / grid_size_) * grid_size_,
                 std::round(point.y() / grid_size_) * grid_size_);
}

void SheetScene::setWireMode(bool enabled) {
  wire_mode_ = enabled;
  cancelWiring();
  emit wireModeChanged(enabled);
  emit statusMessage(enabled ? tr("Wire mode: click the first pin, then the second pin.")
                             : tr("Wire mode off."));
}

void SheetScene::cancelWiring() {
  wire_start_.reset();
  wire_dragging_ = false;
  if (wire_preview_) {
    removeItem(wire_preview_.get());
    wire_preview_.reset();
  }
}

void SheetScene::setSimulationInteractive(bool interactive) {
  simulation_interactive_ = interactive;
  for (ComponentItem* item : std::as_const(component_items_)) {
    item->setSimulationEnabled(interactive);
  }
  if (!interactive) {
    for (WireItem* item : std::as_const(wire_items_)) {
      item->setLive(false);
    }
  } else if (has_simulation_frame_) {
    applySimulationToItems();
  }
}

void SheetScene::setSimulationFrame(const SimFrame& frame) {
  simulation_frame_ = frame;
  has_simulation_frame_ = true;
  if (simulation_interactive_) {
    applySimulationToItems();
  }
}

void SheetScene::clearSimulation() {
  has_simulation_frame_ = false;
  simulation_frame_ = SimFrame{};
  for (ComponentItem* item : std::as_const(component_items_)) {
    item->clearSimulationState();
  }
  for (WireItem* item : std::as_const(wire_items_)) {
    item->setLive(false);
  }
  update();
}

std::optional<SimItemState> SheetScene::simulationState(ComponentId id) const {
  if (!has_simulation_frame_) {
    return std::nullopt;
  }
  const auto iterator = simulation_frame_.states.find(id.value());
  if (iterator == simulation_frame_.states.end()) {
    return std::nullopt;
  }
  return iterator.value();
}

void SheetScene::applySimulationToItems() {
  if (!has_simulation_frame_) {
    return;
  }
  for (ComponentItem* item : std::as_const(component_items_)) {
    const auto iterator = simulation_frame_.states.find(item->componentId().value());
    if (iterator == simulation_frame_.states.end()) {
      item->setSimulationState(SimItemState{});
    } else {
      item->setSimulationState(iterator.value());
    }
  }
  for (WireItem* item : std::as_const(wire_items_)) {
    item->setLive(simulation_frame_.live_wires.contains(item->wireId().value()));
  }
}

ComponentItem* SheetScene::componentItem(ComponentId id) const {
  const auto iterator = component_items_.find(id.value());
  return iterator == component_items_.end() ? nullptr : iterator.value();
}

WireItem* SheetScene::wireItem(WireId id) const {
  const auto iterator = wire_items_.find(id.value());
  return iterator == wire_items_.end() ? nullptr : iterator.value();
}

void SheetScene::requestInput(quint64 component, bool active) {
  emit inputRequested(component, active);
}

void SheetScene::requestTripped(quint64 component, bool tripped) {
  emit tripRequested(component, tripped);
}

void SheetScene::requestSelector(quint64 component, int position) {
  emit selectorRequested(component, position);
}

void SheetScene::requestEmergencyReset() { emit emergencyResetRequested(); }

void SheetScene::requestProperties(ComponentId id) { emit propertiesRequested(id.value()); }

void SheetScene::drawBackground(QPainter* painter, const QRectF& rect) {
  const bool transparent_export = export_mode_ && export_transparent_;
  if (!transparent_export) {
    painter->fillRect(rect, theme_.color(ThemeToken::CanvasBackground));
  }
  const QRectF page = pageRect();
  const fiovivo::Sheet* current = sheet();
  if (current == nullptr) {
    return;
  }

  const QRectF workspace = workspaceRect();
  painter->setRenderHint(QPainter::Antialiasing, false);
  if (!transparent_export && workspace != page) {
    // A larger workspace is drawn as a dashed extension around the paper. Components keep
    // their scene coordinates; only the visible/scrollable drawing area grows.
    QPen workspace_pen(theme_.color(ThemeToken::CanvasBorder), 0.5, Qt::DashLine);
    workspace_pen.setCosmetic(true);
    painter->setPen(workspace_pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(workspace);
  }
  if (!transparent_export) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(theme_.color(ThemeToken::SheetShadow));
    painter->drawRect(page.translated(2.0, 2.0));
    painter->setBrush(theme_.color(ThemeToken::SheetBackground));
  } else {
    painter->setBrush(Qt::NoBrush);
  }
  painter->setPen(QPen(theme_.color(ThemeToken::SheetBorder), 0.4));
  painter->drawRect(page);

  if (grid_visible_ && !export_mode_ && grid_size_ > 0.5) {
    painter->save();
    painter->setClipRect(page);
    const double minor = grid_size_;
    QPen minor_pen(theme_.color(ThemeToken::GridMinor));
    minor_pen.setCosmetic(true);
    minor_pen.setWidthF(0.0);
    QPen major_pen(theme_.color(ThemeToken::GridMajor));
    major_pen.setCosmetic(true);
    major_pen.setWidthF(0.0);
    for (double x = page.left(); x <= page.right(); x += minor) {
      const double index = std::round(x / minor);
      const bool is_major = std::fmod(index, 10.0) == 0.0;
      painter->setPen(is_major ? major_pen : minor_pen);
      painter->drawLine(QPointF(x, page.top()), QPointF(x, page.bottom()));
    }
    for (double y = page.top(); y <= page.bottom(); y += minor) {
      const double index = std::round(y / minor);
      const bool is_major = std::fmod(index, 10.0) == 0.0;
      painter->setPen(is_major ? major_pen : minor_pen);
      painter->drawLine(QPointF(page.left(), y), QPointF(page.right(), y));
    }
    painter->restore();
  }

  if (current->setup.show_frame) {
    const double margin = current->setup.margin;
    const QRectF frame = page.adjusted(margin, margin, -margin, -margin);
    painter->setPen(QPen(theme_.color(ThemeToken::SheetText), 0.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(frame);

    const double block_width = std::min(120.0, frame.width() * 0.45);
    const double block_height = 24.0;
    const QRectF block(
        frame.right() - block_width, frame.bottom() - block_height, block_width, block_height);
    if (!transparent_export) {
      painter->fillRect(block, theme_.color(ThemeToken::TitleBlockBackground));
    }
    painter->drawRect(block);
    QFont title_font(QStringLiteral("Sans Serif"));
    title_font.setPointSizeF(3.5);
    painter->setFont(title_font);

    const double row_height = block_height / 5.0;
    for (int row = 1; row < 5; ++row) {
      painter->drawLine(QPointF(block.left(), block.top() + row * row_height),
                        QPointF(block.right(), block.top() + row * row_height));
    }
    painter->drawLine(QPointF(block.left() + block_width * 0.58, block.top()),
                      QPointF(block.left() + block_width * 0.58, block.bottom()));

    const QString title = QString::fromStdString(document_->diagram().metadata.title);
    const QString author = QString::fromStdString(document_->diagram().metadata.author);
    const QString date = QString::fromStdString(document_->diagram().metadata.modified_utc);
    const QString revision = QString::fromStdString(document_->diagram().metadata.revision);
    const int sheet_number =
        static_cast<int>(std::distance(
            document_->diagram().sheets.begin(),
            std::find_if(document_->diagram().sheets.begin(),
                         document_->diagram().sheets.end(),
                         [this](const Sheet& item) { return item.id == sheet_id_; }))) +
        1;
    const QString sheet_label =
        tr("Sheet %1/%2").arg(sheet_number).arg(document_->diagram().sheets.size());

    const auto draw_field = [this, painter, block, block_width, row_height](
                                int row, const QString& label, const QString& value) {
      const QRectF label_rect(block.left() + 1.0,
                              block.top() + row * row_height + 0.8,
                              block_width * 0.56 - 2.0,
                              row_height - 1.0);
      const QRectF value_rect(block.left() + block_width * 0.58 + 1.0,
                              block.top() + row * row_height + 0.8,
                              block_width * 0.42 - 2.0,
                              row_height - 1.0);
      painter->setPen(theme_.color(ThemeToken::SheetTextMuted));
      painter->drawText(label_rect, Qt::AlignLeft | Qt::AlignVCenter, label);
      painter->setPen(theme_.color(ThemeToken::SheetText));
      painter->drawText(value_rect, Qt::AlignLeft | Qt::AlignVCenter, value);
    };
    // QPainter::drawText does not support eliding; clip manually.
    painter->save();
    painter->setClipRect(block);
    draw_field(0, tr("Title"), title);
    draw_field(1, tr("Author"), author);
    draw_field(2, tr("Date"), date.left(10));
    draw_field(3, tr("Revision"), revision);
    draw_field(4, tr("Sheet"), sheet_label);
    painter->restore();
  }
}

void SheetScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  if (event->button() == Qt::LeftButton && !simulation_interactive_ && !export_mode_) {
    if (PinItem* pin = pinAt(event->scenePos())) {
      const PinRef ref = pinRefFor(pin);
      if (wire_start_.has_value()) {
        if (wire_start_.value() == ref) {
          cancelWiring();
        } else {
          finishWiring(ref);
        }
        event->accept();
        return;
      }
      beginWiring(ref);
      wire_dragging_ = !wire_mode_;
      event->accept();
      return;
    }
    if (wire_start_.has_value()) {
      cancelWiring();
    }
  }
  QGraphicsScene::mousePressEvent(event);
}

void SheetScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  emit cursorMoved(event->scenePos());
  if (wire_start_.has_value()) {
    updateWirePreview(event->scenePos());
  }
  QGraphicsScene::mouseMoveEvent(event);
}

void SheetScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  if (event->button() == Qt::LeftButton && wire_dragging_ && wire_start_.has_value()) {
    if (PinItem* pin = pinAt(event->scenePos())) {
      finishWiring(pinRefFor(pin));
    } else {
      cancelWiring();
    }
    wire_dragging_ = false;
    event->accept();
    return;
  }
  QGraphicsScene::mouseReleaseEvent(event);
  if (event->button() == Qt::LeftButton && !simulation_interactive_) {
    commitMoves();
  }
}

void SheetScene::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    cancelWiring();
    event->accept();
    return;
  }
  QGraphicsScene::keyPressEvent(event);
}

void SheetScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  QMenu menu;
  QAction* paste = menu.addAction(tr("Paste"));
  paste->setEnabled(!clipboard_.empty());
  QAction* select_all = menu.addAction(tr("Select All"));
  menu.addSeparator();
  QAction* add_text = menu.addAction(tr("Add Text Note"));
  QAction* add_rectangle = menu.addAction(tr("Add Rectangle"));
  QAction* chosen = menu.exec(event->screenPos());
  if (chosen == paste) {
    pasteClipboard();
  } else if (chosen == select_all) {
    selectAllContent();
  } else if (chosen == add_text) {
    QPointF position = snap(event->scenePos());
    Annotation annotation = makeTextAnnotation(
        document_->diagram(), Vec2{position.x(), position.y()}, tr("Text note").toStdString());
    AnnotationId created;
    document_->mutate(tr("Add text note"), [&](Diagram& diagram) {
      auto result = diagram.addAnnotation(sheet_id_, annotation);
      if (result) {
        created = result.value();
      }
    });
    for (auto [id, item] : annotation_items_.asKeyValueRange()) {
      (void)id;
      if (item->annotationId() == created) {
        item->setSelected(true);
      }
    }
  } else if (chosen == add_rectangle) {
    QPointF position = snap(event->scenePos());
    Annotation annotation = makeRectangleAnnotation(
        document_->diagram(), Vec2{position.x(), position.y()}, Vec2{60.0, 30.0});
    AnnotationId created;
    document_->mutate(tr("Add rectangle"), [&](Diagram& diagram) {
      auto result = diagram.addAnnotation(sheet_id_, annotation);
      if (result) {
        created = result.value();
      }
    });
    for (auto [id, item] : annotation_items_.asKeyValueRange()) {
      (void)id;
      if (item->annotationId() == created) {
        item->setSelected(true);
      }
    }
  }
  event->accept();
}

void SheetScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
  if (event->mimeData()->hasFormat(kSymbolMimeType)) {
    event->acceptProposedAction();
    return;
  }
  QGraphicsScene::dragEnterEvent(event);
}

void SheetScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
  if (event->mimeData()->hasFormat(kSymbolMimeType)) {
    event->acceptProposedAction();
    return;
  }
  QGraphicsScene::dragMoveEvent(event);
}

void SheetScene::dropEvent(QGraphicsSceneDragDropEvent* event) {
  if (!event->mimeData()->hasFormat(kSymbolMimeType)) {
    QGraphicsScene::dropEvent(event);
    return;
  }
  const QString type = QString::fromUtf8(event->mimeData()->data(kSymbolMimeType));
  addSymbolAt(type.toStdString(), snap(event->scenePos()));
  event->acceptProposedAction();
}

PinItem* SheetScene::pinAt(QPointF scene_position) const {
  const QList<QGraphicsItem*> candidates = items(scene_position, Qt::IntersectsItemShape);
  for (QGraphicsItem* candidate : candidates) {
    if (auto* pin = dynamic_cast<PinItem*>(candidate)) {
      if (pin->contains(pin->mapFromScene(scene_position))) {
        return pin;
      }
    }
  }
  return nullptr;
}

PinRef SheetScene::pinRefFor(const PinItem* pin) const {
  return {pin->componentItem()->componentId(), pin->pinId()};
}

void SheetScene::beginWiring(PinRef start) {
  wire_start_ = start;
  if (!wire_preview_) {
    wire_preview_ = std::make_unique<QGraphicsPathItem>();
    wire_preview_->setZValue(20.0);
    addItem(wire_preview_.get());
  }
  QPen pen(energizedColor(), 0.8, Qt::DashLine);
  pen.setCosmetic(true);
  wire_preview_->setPen(pen);
  ComponentItem* item = componentItem(start.component);
  if (item != nullptr) {
    updateWirePreview(item->pinScenePosition(start.pin));
  }
  emit statusMessage(tr("Select the destination pin. Press Esc to cancel."));
}

void SheetScene::updateWirePreview(QPointF cursor) {
  if (!wire_start_.has_value()) {
    return;
  }
  ComponentItem* item = componentItem(wire_start_->component);
  if (item == nullptr) {
    return;
  }
  const QPointF start = item->pinScenePosition(wire_start_->pin);
  QPainterPath path(start);
  if (std::abs(start.x() - cursor.x()) < 0.01 || std::abs(start.y() - cursor.y()) < 0.01) {
    path.lineTo(cursor);
  } else {
    path.lineTo(cursor.x(), start.y());
    path.lineTo(cursor);
  }
  if (wire_preview_) {
    wire_preview_->setPath(path);
  }
}

void SheetScene::finishWiring(PinRef target) {
  if (!wire_start_.has_value()) {
    return;
  }
  const PinRef start = wire_start_.value();
  cancelWiring();
  if (start == target) {
    return;
  }
  const Component* start_component = document_->diagram().findComponent(start.component);
  const Component* target_component = document_->diagram().findComponent(target.component);
  if (start_component == nullptr || target_component == nullptr) {
    return;
  }
  std::vector<Rect> obstacles;
  for (ComponentItem* item : std::as_const(component_items_)) {
    if (item->componentId() == start.component || item->componentId() == target.component) {
      continue;
    }
    const Component& component = item->component();
    obstacles.push_back(component.boundingBox());
  }
  WireStyle style;
  style.color = wireColor().name().toStdString();
  style.thickness = 0.8;
  WireId created;
  Error wire_error;
  const bool changed = document_->mutate(tr("Create wire"), [&](Diagram& diagram) {
    const auto result = diagram.createWire(sheet_id_, start, target, obstacles, style, false);
    if (!result) {
      wire_error = result.error();
      return;
    }
    created = result.value();
  });
  if (!changed) {
    emit statusMessage(
        tr("Could not create wire: %1").arg(QString::fromStdString(wire_error.toString())));
    return;
  }
  if (WireItem* item = wireItem(created)) {
    clearSelection();
    item->setSelected(true);
  }
}

void SheetScene::commitMoves() {
  std::vector<std::pair<ComponentId, QPointF>> moves;
  for (QGraphicsItem* selected : selectedItems()) {
    auto* item = dynamic_cast<ComponentItem*>(selected);
    if (item == nullptr) {
      continue;
    }
    const Component& component = item->component();
    const QPointF target = snap(item->pos());
    if (std::abs(target.x() - component.position.x) > 0.001 ||
        std::abs(target.y() - component.position.y) > 0.001) {
      moves.emplace_back(component.id, target);
    }
  }
  if (moves.empty()) {
    return;
  }
  QTimer::singleShot(0, this, [this, moves] {
    document_->mutate(tr("Move components"), [&moves](Diagram& diagram) {
      for (const auto& [id, position] : moves) {
        if (Component* component = diagram.findComponent(id)) {
          component->position = {position.x(), position.y()};
        }
      }
    });
  });
}

void SheetScene::addSymbolAt(const std::string& type, QPointF scene_position) {
  const SymbolDefinition* definition = library_->find(type);
  if (definition == nullptr) {
    emit statusMessage(tr("Unknown symbol type: %1").arg(QString::fromStdString(type)));
    return;
  }
  const QPointF position = snap(scene_position);
  ComponentId created;
  document_->mutate(tr("Insert %1").arg(QString::fromStdString(definition->displayName(true))),
                    [&](Diagram& diagram) {
                      Component component = createSymbolComponent(
                          *definition, diagram, Vec2{position.x(), position.y()});
                      auto result = diagram.addComponent(sheet_id_, std::move(component));
                      if (result) {
                        created = result.value();
                      }
                    });
  selectComponent(created);
}

void SheetScene::deleteSelection() {
  const QList<ComponentId> components = selectedComponentIds();
  const QList<WireId> wires = selectedWireIds();
  const QList<AnnotationId> annotations = selectedAnnotationIds();
  if (components.isEmpty() && wires.isEmpty() && annotations.isEmpty()) {
    return;
  }
  document_->mutate(tr("Delete selection"), [&](Diagram& diagram) {
    for (const WireId wire : wires) {
      diagram.removeWire(wire);
    }
    for (const ComponentId component : components) {
      diagram.removeComponent(component);
    }
    if (Sheet* current = diagram.findSheet(sheet_id_)) {
      for (const AnnotationId annotation : annotations) {
        current->removeAnnotation(annotation);
      }
    }
  });
}

void SheetScene::rotateSelection() {
  const QList<ComponentId> components = selectedComponentIds();
  if (components.isEmpty()) {
    return;
  }
  document_->mutate(tr("Rotate components"), [&](Diagram& diagram) {
    for (const ComponentId id : components) {
      if (Component* component = diagram.findComponent(id)) {
        component->rotation = nextClockwise(component->rotation);
      }
    }
  });
}

void SheetScene::copySelection() {
  clipboard_.clear();
  QList<ComponentId> components = selectedComponentIds();
  std::sort(components.begin(), components.end(), [](ComponentId left, ComponentId right) {
    return left < right;
  });
  for (const ComponentId id : components) {
    if (const Component* component = document_->diagram().findComponent(id)) {
      clipboard_.push_back(*component);
    }
  }
  paste_count_ = 0;
  emit statusMessage(clipboard_.empty() ? tr("Nothing selected to copy.")
                                        : tr("Copied %1 component(s).").arg(clipboard_.size()));
}

void SheetScene::pasteClipboard() {
  if (clipboard_.empty()) {
    return;
  }
  ++paste_count_;
  const double offset = 10.0 * paste_count_;
  std::vector<ComponentId> created;
  document_->mutate(tr("Paste"), [&](Diagram& diagram) {
    for (const Component& source : clipboard_) {
      Component component = cloneComponent(source, diagram, Vec2{offset, offset});
      auto result = diagram.addComponent(sheet_id_, std::move(component));
      if (result) {
        created.push_back(result.value());
      }
    }
  });
  clearSelection();
  for (const ComponentId id : created) {
    if (ComponentItem* item = componentItem(id)) {
      item->setSelected(true);
    }
  }
}

void SheetScene::duplicateSelection() {
  copySelection();
  if (!clipboard_.empty()) {
    pasteClipboard();
  }
}

void SheetScene::selectAllContent() {
  for (ComponentItem* item : std::as_const(component_items_)) {
    item->setSelected(true);
  }
  for (WireItem* item : std::as_const(wire_items_)) {
    item->setSelected(true);
  }
  for (AnnotationItem* item : std::as_const(annotation_items_)) {
    item->setSelected(true);
  }
}

void SheetScene::moveSelection(QPointF delta) {
  const QList<ComponentId> components = selectedComponentIds();
  if (components.isEmpty()) {
    return;
  }
  document_->mutate(tr("Move components"), [&](Diagram& diagram) {
    for (const ComponentId id : components) {
      if (Component* component = diagram.findComponent(id)) {
        component->position = {component->position.x + delta.x(),
                               component->position.y + delta.y()};
      }
    }
  });
}

QList<ComponentId> SheetScene::selectedComponentIds() const {
  QList<ComponentId> ids;
  for (QGraphicsItem* item : selectedItems()) {
    if (auto* component = dynamic_cast<ComponentItem*>(item)) {
      ids.push_back(component->componentId());
    }
  }
  return ids;
}

QList<WireId> SheetScene::selectedWireIds() const {
  QList<WireId> ids;
  for (QGraphicsItem* item : selectedItems()) {
    if (auto* wire = dynamic_cast<WireItem*>(item)) {
      ids.push_back(wire->wireId());
    }
  }
  return ids;
}

QList<AnnotationId> SheetScene::selectedAnnotationIds() const {
  QList<AnnotationId> ids;
  for (QGraphicsItem* item : selectedItems()) {
    if (auto* annotation = dynamic_cast<AnnotationItem*>(item)) {
      ids.push_back(annotation->annotationId());
    }
  }
  return ids;
}

void SheetScene::selectComponent(ComponentId id) {
  clearSelection();
  if (ComponentItem* item = componentItem(id)) {
    item->setSelected(true);
  }
}

void SheetScene::selectWire(WireId id) {
  clearSelection();
  if (WireItem* item = wireItem(id)) {
    item->setSelected(true);
  }
}

void SheetScene::setExportMode(bool enabled) {
  export_mode_ = enabled;
  if (enabled) {
    cancelWiring();
  }
  update();
}

void SheetScene::setExportBackgroundTransparent(bool transparent) {
  export_transparent_ = transparent;
  update();
}

void SheetScene::bringSelectionToFront(bool front) {
  const QList<ComponentId> components = selectedComponentIds();
  const Sheet* current = sheet();
  if (components.isEmpty() || current == nullptr) {
    return;
  }
  int extreme = 0;
  bool first = true;
  for (const Component& component : current->components) {
    const int z = static_cast<int>(parameterInt(component.parameters, "z_order", 0));
    if (first) {
      extreme = z;
      first = false;
    } else {
      extreme = front ? std::max(extreme, z) : std::min(extreme, z);
    }
  }
  const int target = front ? extreme + 1 : extreme - 1;
  document_->mutate(tr("Change stacking order"), [&](Diagram& diagram) {
    for (const ComponentId id : components) {
      if (Component* component = diagram.findComponent(id)) {
        component->parameters["z_order"] = static_cast<std::int64_t>(target);
      }
    }
  });
}

void SheetScene::showComponentContextMenu(ComponentItem* item, QPoint screen_position) {
  if (item == nullptr) {
    return;
  }
  if (!item->isSelected()) {
    clearSelection();
    item->setSelected(true);
  }
  QMenu menu;
  QAction* properties = menu.addAction(tr("Properties..."));
  QAction* rotate = menu.addAction(tr("Rotate 90° Clockwise"));
  QAction* duplicate = menu.addAction(tr("Duplicate"));
  QAction* remove = menu.addAction(tr("Delete"));
  menu.addSeparator();
  QAction* front = menu.addAction(tr("Bring to Front"));
  QAction* back = menu.addAction(tr("Send to Back"));

  QAction* simulation_action = nullptr;
  const Component& component = item->component();
  const std::optional<SimItemState> state = simulationState(component.id);
  if (simulation_interactive_ && state.has_value()) {
    menu.addSeparator();
    if (component.type == component_type::kEmergencyStop) {
      simulation_action = menu.addAction(tr("Reset Emergency Stop"));
    } else if (component.type == component_type::kFuse ||
               component.type == component_type::kBreaker ||
               component.type == component_type::kThermalRelay) {
      simulation_action = menu.addAction(state->tripped ? tr("Reset Device") : tr("Trip Device"));
    } else if (component.type == component_type::kSelectorSwitch) {
      QMenu* positions = menu.addMenu(tr("Selector Position"));
      const int count =
          std::max(1, static_cast<int>(parameterInt(component.parameters, "positions", 2)));
      for (int position = 1; position <= count; ++position) {
        QAction* action = positions->addAction(tr("Position %1").arg(position));
        action->setCheckable(true);
        action->setChecked(position == state->selector_position);
        action->setData(position);
      }
      QAction* chosen = menu.exec(screen_position);
      if (chosen != nullptr && chosen->parent() == positions) {
        requestSelector(component.id.value(), chosen->data().toInt());
      }
      return;
    }
  }

  QAction* chosen = menu.exec(screen_position);
  if (chosen == properties) {
    requestProperties(component.id);
  } else if (chosen == rotate) {
    rotateSelection();
  } else if (chosen == duplicate) {
    duplicateSelection();
  } else if (chosen == remove) {
    deleteSelection();
  } else if (chosen == front) {
    bringSelectionToFront(true);
  } else if (chosen == back) {
    bringSelectionToFront(false);
  } else if (chosen != nullptr && chosen == simulation_action) {
    if (component.type == component_type::kEmergencyStop) {
      requestInput(component.id.value(), false);
    } else {
      requestTripped(component.id.value(), !state->tripped);
    }
  }
}

void SheetScene::showWireContextMenu(WireItem* item, QPoint screen_position) {
  if (!item->isSelected()) {
    clearSelection();
    item->setSelected(true);
  }
  QMenu menu;
  QAction* remove = menu.addAction(tr("Delete Wire"));
  QAction* chosen = menu.exec(screen_position);
  if (chosen == remove) {
    deleteSelection();
  }
}

void SheetScene::showAnnotationContextMenu(AnnotationItem* item, QPoint screen_position) {
  if (!item->isSelected()) {
    clearSelection();
    item->setSelected(true);
  }
  QMenu menu;
  QAction* remove = menu.addAction(tr("Delete Annotation"));
  QAction* chosen = menu.exec(screen_position);
  if (chosen == remove) {
    deleteSelection();
  }
}

} // namespace fiovivo::ui
