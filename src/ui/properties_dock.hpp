#pragma once

#include "core/ids.hpp"
#include "core/types.hpp"
#include "ui/sim_ui_types.hpp"

#include <QWidget>

class QScrollArea;
class QVBoxLayout;
class QLabel;

namespace fiovivo {
class SymbolLibrary;
} // namespace fiovivo

namespace fiovivo::ui {

class Document;
class SheetScene;

class PropertiesDock : public QWidget {
  Q_OBJECT

public:
  PropertiesDock(Document* document, const SymbolLibrary* library, QWidget* parent = nullptr);

  void setScene(SheetScene* scene);
  void updateSimulation(const SimFrame& frame);

private:
  void scheduleRefresh();
  void refresh();
  void buildEmpty();
  void buildComponent(ComponentId id, QVBoxLayout* layout);
  void buildWire(WireId id, QVBoxLayout* layout);
  void buildAnnotation(AnnotationId id, QVBoxLayout* layout);
  void applyParameter(ComponentId component, const std::string& key, const ParameterValue& value);
  [[nodiscard]] bool isPortuguese() const;

  Document* document_ = nullptr;
  const SymbolLibrary* library_ = nullptr;
  SheetScene* scene_ = nullptr;
  QScrollArea* scroll_ = nullptr;
  QLabel* simulation_label_ = nullptr;
  QLabel* wire_live_label_ = nullptr;
  ComponentId current_component_;
  WireId current_wire_;
  AnnotationId current_annotation_;
  SimFrame simulation_frame_;
  bool has_simulation_frame_ = false;
  bool refresh_scheduled_ = false;
  bool updating_ = false;
};

} // namespace fiovivo::ui
