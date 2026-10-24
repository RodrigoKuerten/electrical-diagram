#pragma once

#include "ui/theme.hpp"

#include <QIcon>
#include <QWidget>

#include <string_view>

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

  void setThemePalette(const ThemePalette& palette);

signals:
  void symbolActivated(const QString& type);

private:
  void buildTree();
  void filter(const QString& text);
  void updateIcons();
  [[nodiscard]] QIcon symbolIcon(std::string_view type) const;
  [[nodiscard]] bool isPortuguese() const;

  const SymbolLibrary& library_;
  SymbolRenderCache& render_cache_;
  ThemePalette palette_ = lightTheme();
  QLineEdit* search_ = nullptr;
  QTreeWidget* tree_ = nullptr;
};

} // namespace fiovivo::ui
