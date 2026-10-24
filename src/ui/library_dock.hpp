#pragma once

#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace fiovivo {
class SymbolLibrary;
class SymbolRenderCache;
} // namespace fiovivo

namespace fiovivo::ui {

class LibraryDock : public QWidget {
  Q_OBJECT

public:
  LibraryDock(const SymbolLibrary& library,
              SymbolRenderCache& render_cache,
              QWidget* parent = nullptr);

signals:
  void symbolActivated(const QString& type);

private:
  void buildTree();
  void filter(const QString& text);
  [[nodiscard]] bool isPortuguese() const;

  const SymbolLibrary& library_;
  SymbolRenderCache& render_cache_;
  QLineEdit* search_ = nullptr;
  QTreeWidget* tree_ = nullptr;
};

} // namespace fiovivo::ui
