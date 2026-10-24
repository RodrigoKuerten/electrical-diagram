#include "ui/document.hpp"

#include <QUndoCommand>
#include <QUndoStack>

#include <utility>

namespace fiovivo::ui {
namespace {

[[nodiscard]] std::string preferredSheetName(const Diagram& diagram) {
  return "Sheet " + std::to_string(diagram.sheets.size() + 1);
}

} // namespace

class DiagramSnapshotCommand : public QUndoCommand {
public:
  DiagramSnapshotCommand(Document* document, const QString& text, Diagram before, Diagram after)
      : QUndoCommand(text), document_(document), before_(std::move(before)),
        after_(std::move(after)) {}

  void undo() override {
    document_->diagram_ = before_;
    document_->afterCommand();
  }

  void redo() override {
    document_->diagram_ = after_;
    document_->afterCommand();
  }

private:
  Document* document_;
  Diagram before_;
  Diagram after_;
};

Document::Document(QObject* parent) : QObject(parent), undo_stack_(std::make_unique<QUndoStack>()) {
  undo_stack_->setUndoLimit(250);
  connect(undo_stack_.get(), &QUndoStack::cleanChanged, this, [this](bool clean) {
    (void)clean;
    updateCloud();
  });
  connect(undo_stack_.get(), &QUndoStack::indexChanged, this, [this](int index) {
    (void)index;
    emit undoStateChanged();
  });
  clear();
}

Document::~Document() = default;

const Sheet* Document::currentSheet() const { return diagram_.findSheet(current_sheet_); }
Sheet* Document::currentSheet() { return diagram_.findSheet(current_sheet_); }

void Document::setCurrentSheet(SheetId id) {
  if (current_sheet_ == id) {
    return;
  }
  if (diagram_.findSheet(id) == nullptr) {
    return;
  }
  current_sheet_ = id;
  emit currentSheetChanged();
  updateCloud();
}

bool Document::isModified() const { return force_modified_ || !undo_stack_->isClean(); }

void Document::setModified(bool modified) {
  force_modified_ = modified;
  if (!modified) {
    undo_stack_->setClean();
  }
  updateCloud();
}

void Document::setFilePath(const QString& path, bool mark_clean) {
  file_path_ = path;
  if (mark_clean) {
    undo_stack_->setClean();
  }
  emit filePathChanged(path);
  updateCloud();
}

void Document::markClean() {
  force_modified_ = false;
  undo_stack_->setClean();
  updateCloud();
}

QString Document::title() const {
  if (!diagram_.metadata.title.empty()) {
    return QString::fromStdString(diagram_.metadata.title);
  }
  return QStringLiteral("Untitled");
}

void Document::setDiagram(Diagram diagram, bool mark_clean) {
  diagram_ = std::move(diagram);
  if (diagram_.sheets.empty()) {
    Sheet sheet;
    sheet.id = diagram_.ids().newSheetId();
    sheet.name = preferredSheetName(diagram_);
    const auto added = diagram_.addSheet(std::move(sheet));
    (void)added;
  }
  current_sheet_ = diagram_.sheets.front().id;
  undo_stack_->clear();
  file_path_.clear();
  force_modified_ = false;
  if (mark_clean) {
    undo_stack_->setClean();
  }
  emit diagramChanged();
  emit currentSheetChanged();
  emit undoStateChanged();
  emit filePathChanged(file_path_);
  updateCloud();
}

void Document::clear() {
  Diagram diagram;
  Sheet sheet;
  sheet.id = diagram.ids().newSheetId();
  sheet.name = tr("Sheet 1").toStdString();
  (void)diagram.addSheet(std::move(sheet));
  setDiagram(std::move(diagram));
  file_path_.clear();
  undo_stack_->setClean();
  updateCloud();
}

bool Document::mutate(const QString& text, const std::function<void(Diagram&)>& mutation) {
  Diagram before = diagram_;
  Diagram after = diagram_;
  mutation(after);
  if (after == before) {
    return false;
  }
  auto command =
      std::make_unique<DiagramSnapshotCommand>(this, text, std::move(before), std::move(after));
  undo_stack_->push(command.release());
  return true;
}

void Document::undo() { undo_stack_->undo(); }

void Document::redo() { undo_stack_->redo(); }

bool Document::canUndo() const { return undo_stack_->canUndo(); }
bool Document::canRedo() const { return undo_stack_->canRedo(); }
QString Document::undoText() const { return undo_stack_->undoText(); }
QString Document::redoText() const { return undo_stack_->redoText(); }

void Document::afterCommand() {
  force_modified_ = false;
  if (diagram_.findSheet(current_sheet_) == nullptr && !diagram_.sheets.empty()) {
    current_sheet_ = diagram_.sheets.front().id;
    emit currentSheetChanged();
  }
  emit diagramChanged();
  emit undoStateChanged();
  updateCloud();
}

void Document::updateCloud() {
  const bool modified = isModified();
  if (modified != last_modified_) {
    last_modified_ = modified;
    emit modifiedChanged(modified);
  }
}

} // namespace fiovivo::ui
