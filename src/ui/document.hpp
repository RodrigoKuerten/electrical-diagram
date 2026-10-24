#pragma once

#include "core/model.hpp"
#include "core/result.hpp"

#include <QObject>
#include <QString>

#include <functional>
#include <memory>
#include <string>

class QUndoStack;

namespace fiovivo::ui {

class Document : public QObject {
  Q_OBJECT

public:
  explicit Document(QObject* parent = nullptr);
  ~Document() override;

  [[nodiscard]] const Diagram& diagram() const { return diagram_; }
  [[nodiscard]] Diagram& diagram() { return diagram_; }
  [[nodiscard]] const Sheet* currentSheet() const;
  [[nodiscard]] Sheet* currentSheet();
  [[nodiscard]] SheetId currentSheetId() const { return current_sheet_; }
  void setCurrentSheet(SheetId id);

  [[nodiscard]] bool isModified() const;
  void setModified(bool modified);
  [[nodiscard]] QString filePath() const { return file_path_; }
  void setFilePath(const QString& path, bool mark_clean);
  void markClean();
  [[nodiscard]] QString title() const;

  void setDiagram(Diagram diagram, bool mark_clean = true);
  void clear();

  // Applies a mutation as one undoable snapshot. Returns false when nothing changed.
  bool mutate(const QString& text, const std::function<void(Diagram&)>& mutation);
  void undo();
  void redo();
  [[nodiscard]] bool canUndo() const;
  [[nodiscard]] bool canRedo() const;
  [[nodiscard]] QString undoText() const;
  [[nodiscard]] QString redoText() const;

  [[nodiscard]] QUndoStack* undoStack() { return undo_stack_.get(); }

signals:
  void diagramChanged();
  void modifiedChanged(bool modified);
  void filePathChanged(const QString& path);
  void currentSheetChanged();
  void undoStateChanged();

private:
  friend class DiagramSnapshotCommand;
  void afterCommand();
  void updateCloud();

  Diagram diagram_;
  SheetId current_sheet_;
  QString file_path_;
  std::unique_ptr<QUndoStack> undo_stack_;
  bool last_modified_ = false;
  bool force_modified_ = false;
};

} // namespace fiovivo::ui
