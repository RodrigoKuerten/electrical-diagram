#include "ui/properties_dock.hpp"

#include "core/model.hpp"
#include "core/types.hpp"
#include "library/symbol_definition.hpp"
#include "ui/document.hpp"
#include "ui/properties_dock.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/ui_helpers.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <variant>

namespace fiovivo::ui {
namespace {

[[nodiscard]] QDoubleSpinBox* makeLengthSpin(QWidget* parent) {
  auto* spin = new QDoubleSpinBox(parent);
  spin->setDecimals(2);
  spin->setRange(-10000.0, 10000.0);
  spin->setSingleStep(1.0);
  spin->setSuffix(QStringLiteral(" mm"));
  spin->setKeyboardTracking(false);
  return spin;
}

template<typename T>
[[nodiscard]] T valueOr(const ParameterMap& map, const std::string& key, T fallback) {
  const auto iterator = map.find(key);
  if (iterator == map.end()) {
    return fallback;
  }
  if (const auto* value = std::get_if<T>(&iterator->second)) {
    return *value;
  }
  return fallback;
}

} // namespace

PropertiesDock::PropertiesDock(Document* document, const SymbolLibrary* library, QWidget* parent)
    : QWidget(parent), document_(document), library_(library) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  scroll_ = new QScrollArea(this);
  scroll_->setWidgetResizable(true);
  layout->addWidget(scroll_);
  buildEmpty();
  connect(document_, &Document::diagramChanged, this, &PropertiesDock::scheduleRefresh);
}

void PropertiesDock::setScene(SheetScene* scene) {
  if (scene_ == scene) {
    return;
  }
  if (scene_ != nullptr) {
    disconnect(scene_, nullptr, this, nullptr);
  }
  scene_ = scene;
  if (scene_ != nullptr) {
    connect(scene_, &QGraphicsScene::selectionChanged, this, &PropertiesDock::scheduleRefresh);
  }
  scheduleRefresh();
}

void PropertiesDock::scheduleRefresh() {
  if (refresh_scheduled_) {
    return;
  }
  refresh_scheduled_ = true;
  QTimer::singleShot(0, this, [this] {
    refresh_scheduled_ = false;
    refresh();
  });
}

bool PropertiesDock::isPortuguese() const { return QLocale().language() == QLocale::Portuguese; }

void PropertiesDock::buildEmpty() {
  auto* content = new QWidget();
  auto* layout = new QVBoxLayout(content);
  auto* label =
      new QLabel(tr("Select a component, wire or annotation to edit its properties."), content);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  layout->addWidget(label);
  layout->addStretch(1);
  scroll_->setWidget(content);
  current_component_ = {};
  current_wire_ = {};
  current_annotation_ = {};
  simulation_label_ = nullptr;
  wire_live_label_ = nullptr;
}

void PropertiesDock::refresh() {
  if (scene_ == nullptr || document_ == nullptr) {
    buildEmpty();
    return;
  }
  const QList<ComponentId> components = scene_->selectedComponentIds();
  const QList<WireId> wires = scene_->selectedWireIds();
  const QList<AnnotationId> annotations = scene_->selectedAnnotationIds();
  const int total = static_cast<int>(components.size() + wires.size() + annotations.size());
  if (total == 0) {
    buildEmpty();
    return;
  }

  simulation_label_ = nullptr;
  wire_live_label_ = nullptr;
  current_component_ = {};
  current_wire_ = {};
  current_annotation_ = {};

  auto* content = new QWidget();
  auto* layout = new QVBoxLayout(content);
  layout->setContentsMargins(8, 8, 8, 8);
  if (total > 1) {
    auto* label = new QLabel(tr("%1 component(s), %2 wire(s), %3 annotation(s) selected.")
                                 .arg(static_cast<int>(components.size()))
                                 .arg(static_cast<int>(wires.size()))
                                 .arg(static_cast<int>(annotations.size())),
                             content);
    label->setWordWrap(true);
    layout->addWidget(label);
    auto* hint = new QLabel(tr("Select a single item to edit its properties."), content);
    hint->setWordWrap(true);
    layout->addWidget(hint);
    layout->addStretch(1);
    scroll_->setWidget(content);
    return;
  }
  if (!components.isEmpty()) {
    buildComponent(components.front(), layout);
    current_component_ = components.front();
    current_wire_ = {};
    current_annotation_ = {};
  } else if (!wires.isEmpty()) {
    buildWire(wires.front(), layout);
    current_component_ = {};
    current_wire_ = wires.front();
    current_annotation_ = {};
  } else if (!annotations.isEmpty()) {
    buildAnnotation(annotations.front(), layout);
    current_component_ = {};
    current_wire_ = {};
    current_annotation_ = annotations.front();
  }
  layout->addStretch(1);
  scroll_->setWidget(content);
  updateSimulation(simulation_frame_);
}

void PropertiesDock::buildComponent(ComponentId id, QVBoxLayout* layout) {
  const Component* component = document_->diagram().findComponent(id);
  if (component == nullptr) {
    return;
  }
  const SymbolDefinition* definition = library_->find(component->type);
  auto* heading = new QLabel(componentTypeText(component->type, library_, isPortuguese()));
  QFont heading_font = heading->font();
  heading_font.setBold(true);
  heading_font.setPointSizeF(heading_font.pointSizeF() + 1.5);
  heading->setFont(heading_font);
  layout->addWidget(heading);
  layout->addWidget(new QLabel(QString::fromStdString(component->type), this));

  auto* form = new QFormLayout();
  auto* tag = new QLineEdit(QString::fromStdString(component->tag));
  auto* description = new QLineEdit(QString::fromStdString(component->description));
  auto* x = makeLengthSpin(this);
  auto* y = makeLengthSpin(this);
  x->setValue(component->position.x);
  y->setValue(component->position.y);
  auto* rotation = new QComboBox();
  rotation->addItems({"0°", "90°", "180°", "270°"});
  rotation->setCurrentIndex(rotationDegrees(component->rotation) / 90);
  form->addRow(tr("Tag"), tag);
  form->addRow(tr("Description"), description);
  form->addRow(tr("X"), x);
  form->addRow(tr("Y"), y);
  form->addRow(tr("Rotation"), rotation);
  simulation_label_ = new QLabel();
  simulation_label_->setWordWrap(true);
  form->addRow(tr("Simulation"), simulation_label_);
  layout->addLayout(form);

  const auto apply_position = [this, id, x, y] {
    if (updating_) {
      return;
    }
    document_->mutate(tr("Move component"), [id, x, y](Diagram& diagram) {
      if (Component* item = diagram.findComponent(id)) {
        item->position = {x->value(), y->value()};
      }
    });
  };
  connect(x, &QDoubleSpinBox::editingFinished, this, apply_position);
  connect(y, &QDoubleSpinBox::editingFinished, this, apply_position);
  connect(tag, &QLineEdit::editingFinished, this, [this, id, tag] {
    if (updating_) {
      return;
    }
    const QString text = tag->text().trimmed();
    if (text.isEmpty()) {
      QMessageBox::warning(this, tr("Invalid tag"), tr("The tag must not be empty."));
      scheduleRefresh();
      return;
    }
    if (document_->diagram().isTagUsed(text.toStdString(), id)) {
      QMessageBox::warning(this,
                           tr("Duplicate tag"),
                           tr("The tag \"%1\" is already used in this project.").arg(text));
      scheduleRefresh();
      return;
    }
    document_->mutate(tr("Change tag"), [id, text](Diagram& diagram) {
      if (Component* item = diagram.findComponent(id)) {
        item->tag = text.toStdString();
      }
    });
  });
  connect(description, &QLineEdit::editingFinished, this, [this, id, description] {
    if (updating_) {
      return;
    }
    document_->mutate(tr("Change description"), [id, description](Diagram& diagram) {
      if (Component* item = diagram.findComponent(id)) {
        item->description = description->text().toStdString();
      }
    });
  });
  connect(rotation, &QComboBox::currentIndexChanged, this, [this, id](int index) {
    if (updating_) {
      return;
    }
    const Rotation value = static_cast<Rotation>((index * 90) % 360);
    document_->mutate(tr("Rotate component"), [id, value](Diagram& diagram) {
      if (Component* item = diagram.findComponent(id)) {
        item->rotation = value;
      }
    });
  });

  auto* buttons = new QHBoxLayout();
  auto* rotate = new QPushButton(tr("Rotate"), this);
  auto* duplicate = new QPushButton(tr("Duplicate"), this);
  auto* remove = new QPushButton(tr("Delete"), this);
  buttons->addWidget(rotate);
  buttons->addWidget(duplicate);
  buttons->addWidget(remove);
  layout->addLayout(buttons);
  connect(rotate, &QPushButton::clicked, this, [this] { scene_->rotateSelection(); });
  connect(duplicate, &QPushButton::clicked, this, [this] { scene_->duplicateSelection(); });
  connect(remove, &QPushButton::clicked, this, [this] { scene_->deleteSelection(); });

  if (definition != nullptr && !definition->parameter_definitions.empty()) {
    auto* group = new QGroupBox(tr("Parameters"), this);
    auto* group_form = new QFormLayout(group);
    std::vector<std::string> handled;
    for (const ParameterDefinition& parameter : definition->parameter_definitions) {
      if (parameter.key == "z_order") {
        continue;
      }
      handled.push_back(parameter.key);
      const QString label = QString::fromStdString(parameter.displayLabel(isPortuguese()));
      const QString unit = QString::fromStdString(parameter.unit);
      switch (parameter.kind) {
      case ParameterKind::Boolean: {
        auto* check = new QCheckBox(group);
        check->setChecked(parameterBool(component->parameters, parameter.key, false));
        connect(check, &QCheckBox::toggled, this, [this, id, key = parameter.key](bool checked) {
          if (!updating_) {
            applyParameter(id, key, ParameterValue{checked});
          }
        });
        group_form->addRow(label, check);
        break;
      }
      case ParameterKind::Integer: {
        auto* spin = new QSpinBox(group);
        spin->setRange(-1000000000, 1000000000);
        if (parameter.minimum.has_value()) {
          spin->setMinimum(static_cast<int>(*parameter.minimum));
        }
        if (parameter.maximum.has_value()) {
          spin->setMaximum(static_cast<int>(*parameter.maximum));
        }
        spin->setValue(static_cast<int>(parameterInt(component->parameters, parameter.key, 0)));
        if (!unit.isEmpty()) {
          spin->setSuffix(" " + unit);
        }
        spin->setKeyboardTracking(false);
        connect(spin, &QSpinBox::editingFinished, this, [this, id, key = parameter.key, spin] {
          if (!updating_) {
            applyParameter(id, key, ParameterValue{static_cast<std::int64_t>(spin->value())});
          }
        });
        group_form->addRow(label, spin);
        break;
      }
      case ParameterKind::Number: {
        auto* spin = new QDoubleSpinBox(group);
        spin->setDecimals(3);
        spin->setRange(-1000000000.0, 1000000000.0);
        if (parameter.minimum.has_value()) {
          spin->setMinimum(*parameter.minimum);
        }
        if (parameter.maximum.has_value()) {
          spin->setMaximum(*parameter.maximum);
        }
        spin->setValue(parameterNumber(component->parameters, parameter.key, 0.0));
        if (!unit.isEmpty()) {
          spin->setSuffix(" " + unit);
        }
        spin->setKeyboardTracking(false);
        connect(
            spin, &QDoubleSpinBox::editingFinished, this, [this, id, key = parameter.key, spin] {
              if (!updating_) {
                applyParameter(id, key, ParameterValue{spin->value()});
              }
            });
        group_form->addRow(label, spin);
        break;
      }
      case ParameterKind::Enum: {
        auto* combo = new QComboBox(group);
        for (const std::string& option : parameter.options) {
          combo->addItem(QString::fromStdString(option));
        }
        const std::string current =
            parameterString(component->parameters,
                            parameter.key,
                            parameter.options.empty() ? std::string{} : parameter.options.front());
        const int index = combo->findText(QString::fromStdString(current));
        if (index >= 0) {
          combo->setCurrentIndex(index);
        }
        connect(combo,
                &QComboBox::currentTextChanged,
                this,
                [this, id, key = parameter.key](const QString& text) {
                  if (!updating_) {
                    applyParameter(id, key, ParameterValue{text.toStdString()});
                  }
                });
        group_form->addRow(label, combo);
        break;
      }
      case ParameterKind::String: {
        auto* edit = new QLineEdit(
            QString::fromStdString(parameterString(component->parameters, parameter.key)), group);
        connect(edit, &QLineEdit::editingFinished, this, [this, id, key = parameter.key, edit] {
          if (!updating_) {
            applyParameter(id, key, ParameterValue{edit->text().toStdString()});
          }
        });
        group_form->addRow(label, edit);
        break;
      }
      }
    }
    // Preserve and edit parameters that are not declared by the manifest.
    for (const auto& [key, value] : component->parameters) {
      if (key == "z_order" || std::find(handled.begin(), handled.end(), key) != handled.end()) {
        continue;
      }
      auto* edit = new QLineEdit(QString::fromStdString([](const ParameterValue& v) {
                                   if (const auto* b = std::get_if<bool>(&v)) {
                                     return *b ? std::string("true") : std::string("false");
                                   }
                                   if (const auto* i = std::get_if<std::int64_t>(&v)) {
                                     return std::to_string(*i);
                                   }
                                   if (const auto* d = std::get_if<double>(&v)) {
                                     return std::to_string(*d);
                                   }
                                   return std::get<std::string>(v);
                                 }(value)),
                                 group);
      group_form->addRow(QString::fromStdString(key), edit);
      connect(edit, &QLineEdit::editingFinished, this, [this, id, key, edit] {
        if (updating_) {
          return;
        }
        const Component* item = document_->diagram().findComponent(id);
        if (item == nullptr) {
          return;
        }
        if (const ParameterValue* current = findParameter(item->parameters, key)) {
          if (std::holds_alternative<bool>(*current)) {
            applyParameter(
                id, key, ParameterValue{edit->text().compare("true", Qt::CaseInsensitive) == 0});
          } else if (std::holds_alternative<std::int64_t>(*current)) {
            applyParameter(
                id, key, ParameterValue{static_cast<std::int64_t>(edit->text().toLongLong())});
          } else if (std::holds_alternative<double>(*current)) {
            applyParameter(id, key, ParameterValue{edit->text().toDouble()});
          } else {
            applyParameter(id, key, ParameterValue{edit->text().toStdString()});
          }
        }
      });
    }
    layout->addWidget(group);
  }

  if (!component->pins.empty()) {
    auto* pins = new QTableWidget(static_cast<int>(component->pins.size()), 3, this);
    pins->setHorizontalHeaderLabels({tr("Pin"), tr("Role"), tr("Wires")});
    pins->horizontalHeader()->setStretchLastSection(true);
    pins->verticalHeader()->setVisible(false);
    pins->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int row = 0; row < static_cast<int>(component->pins.size()); ++row) {
      const Pin& pin = component->pins[static_cast<std::size_t>(row)];
      pins->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pin.name)));
      pins->setItem(row, 1, new QTableWidgetItem(pinRoleText(pin.role)));
      pins->setItem(row, 2, new QTableWidgetItem(QString::number(pin.wires.size())));
    }
    pins->resizeColumnsToContents();
    layout->addWidget(pins);
  }
}

void PropertiesDock::buildWire(WireId id, QVBoxLayout* layout) {
  const Wire* wire = document_->diagram().findWire(id);
  if (wire == nullptr) {
    return;
  }
  const Sheet* sheet = document_->diagram().findSheetForComponent(wire->from.component);
  if (sheet == nullptr) {
    return;
  }
  auto* heading = new QLabel(tr("Wire"), this);
  QFont heading_font = heading->font();
  heading_font.setBold(true);
  heading->setFont(heading_font);
  layout->addWidget(heading);
  layout->addWidget(
      new QLabel(tr("%1 → %2").arg(QString::fromStdString(endpointName(*sheet, wire->from)),
                                   QString::fromStdString(endpointName(*sheet, wire->to))),
                 this));
  auto* form = new QFormLayout();
  auto* color = new QPushButton(QString::fromStdString(wire->style.color), this);
  color->setStyleSheet(QStringLiteral("background-color:%1; color: white;")
                           .arg(QString::fromStdString(wire->style.color)));
  auto* thickness = new QDoubleSpinBox(this);
  thickness->setRange(0.1, 10.0);
  thickness->setDecimals(2);
  thickness->setSingleStep(0.1);
  thickness->setValue(wire->style.thickness);
  auto* dash = new QComboBox(this);
  dash->addItem(tr("Solid"), "solid");
  dash->addItem(tr("Dashed"), "dashed");
  dash->addItem(tr("Dotted"), "dotted");
  dash->setCurrentIndex(
      dash->findData(QString::fromStdString(std::string(toString(wire->style.dash)))));
  wire_live_label_ = new QLabel(tr("Not simulated"), this);
  form->addRow(tr("Color"), color);
  form->addRow(tr("Thickness"), thickness);
  form->addRow(tr("Dash"), dash);
  form->addRow(tr("Energy"), wire_live_label_);
  layout->addLayout(form);

  const auto apply_style = [this, id, color, thickness, dash] {
    document_->mutate(tr("Edit wire style"), [id, color, thickness, dash](Diagram& diagram) {
      if (Wire* item = diagram.findWire(id)) {
        item->style.color = color->text().toStdString();
        item->style.thickness = thickness->value();
        const auto parsed = wireDashFromString(dash->currentData().toString().toStdString());
        if (parsed.has_value()) {
          item->style.dash = *parsed;
        }
      }
    });
  };
  connect(color, &QPushButton::clicked, this, [this, color, apply_style] {
    const QColor chosen = QColorDialog::getColor(QColor(color->text()), this, tr("Wire color"));
    if (chosen.isValid()) {
      color->setText(chosen.name());
      color->setStyleSheet(QStringLiteral("background-color:%1; color: white;").arg(chosen.name()));
      apply_style();
    }
  });
  connect(thickness, &QDoubleSpinBox::editingFinished, this, apply_style);
  connect(dash, &QComboBox::currentIndexChanged, this, [apply_style](int index) {
    (void)index;
    apply_style();
  });

  auto* remove = new QPushButton(tr("Delete Wire"), this);
  layout->addWidget(remove);
  connect(remove, &QPushButton::clicked, this, [this] { scene_->deleteSelection(); });
}

void PropertiesDock::buildAnnotation(AnnotationId id, QVBoxLayout* layout) {
  const Sheet* sheet = document_->currentSheet();
  if (sheet == nullptr) {
    return;
  }
  const Annotation* annotation = nullptr;
  for (const Annotation& candidate : sheet->annotations) {
    if (candidate.id == id) {
      annotation = &candidate;
      break;
    }
  }
  if (annotation == nullptr) {
    return;
  }
  auto* heading = new QLabel(tr("Annotation"), this);
  QFont heading_font = heading->font();
  heading_font.setBold(true);
  heading->setFont(heading_font);
  layout->addWidget(heading);
  auto* form = new QFormLayout();
  auto* text = new QLineEdit(QString::fromStdString(annotation->text), this);
  auto* font_size = new QDoubleSpinBox(this);
  font_size->setRange(1.0, 40.0);
  font_size->setDecimals(1);
  font_size->setValue(annotation->font_size);
  form->addRow(tr("Text"), text);
  form->addRow(tr("Font size"), font_size);
  layout->addLayout(form);
  connect(text, &QLineEdit::editingFinished, this, [this, id, text] {
    document_->mutate(tr("Edit annotation"), [this, id, text](Diagram& diagram) {
      if (Sheet* current = diagram.findSheet(document_->currentSheetId())) {
        for (Annotation& candidate : current->annotations) {
          if (candidate.id == id) {
            candidate.text = text->text().toStdString();
            break;
          }
        }
      }
    });
  });
  connect(font_size, &QDoubleSpinBox::editingFinished, this, [this, id, font_size] {
    document_->mutate(tr("Edit annotation"), [this, id, font_size](Diagram& diagram) {
      if (Sheet* current = diagram.findSheet(document_->currentSheetId())) {
        for (Annotation& candidate : current->annotations) {
          if (candidate.id == id) {
            candidate.font_size = font_size->value();
            break;
          }
        }
      }
    });
  });
  auto* remove = new QPushButton(tr("Delete Annotation"), this);
  layout->addWidget(remove);
  connect(remove, &QPushButton::clicked, this, [this] { scene_->deleteSelection(); });
}

void PropertiesDock::applyParameter(ComponentId component,
                                    const std::string& key,
                                    const ParameterValue& value) {
  document_->mutate(tr("Edit parameter"), [component, key, value](Diagram& diagram) {
    if (Component* item = diagram.findComponent(component)) {
      item->parameters[key] = value;
    }
  });
}

void PropertiesDock::updateSimulation(const SimFrame& frame) {
  simulation_frame_ = frame;
  has_simulation_frame_ = !frame.states.isEmpty() || frame.time_ms != 0;
  if (simulation_label_ != nullptr && current_component_.valid()) {
    const Component* component = document_->diagram().findComponent(current_component_);
    if (component != nullptr) {
      const auto iterator = frame.states.find(current_component_.value());
      if (iterator != frame.states.end()) {
        const SimItemState state = iterator.value();
        QString text = componentStateText(*component, state);
        if (component->type == component_type::kTimerOn ||
            component->type == component_type::kTimerOff) {
          const std::int64_t delay = parameterInt(component->parameters, "delay_ms", 0);
          text += QStringLiteral(" — %1 / %2 ms").arg(state.timer_elapsed_ms).arg(delay);
        }
        simulation_label_->setText(text);
      } else {
        simulation_label_->setText(tr("No simulation state"));
      }
    }
  }
  if (wire_live_label_ != nullptr && current_wire_.valid()) {
    wire_live_label_->setText(
        frame.live_wires.contains(current_wire_.value()) ? tr("Energized") : tr("De-energized"));
  }
}

} // namespace fiovivo::ui
