#include "ui/library_dock.hpp"

#include "library/svg_renderer.hpp"
#include "library/symbol_definition.hpp"
#include "ui/ui_helpers.hpp"

#include <QDrag>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QLocale>
#include <QMimeData>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace fiovivo::ui {
namespace {

constexpr const char* kSymbolMimeType = "application/x-fiovivo-symbol";

class SymbolTree : public QTreeWidget {
public:
  explicit SymbolTree(QWidget* parent = nullptr) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setIconSize(QSize(46, 46));
    setUniformRowHeights(false);
    setTextElideMode(Qt::ElideNone);
  }

protected:
  void startDrag(Qt::DropActions supported_actions) override {
    (void)supported_actions;
    QTreeWidgetItem* item = currentItem();
    if (item == nullptr || item->data(0, Qt::UserRole).toString().isEmpty()) {
      return;
    }
    auto mime = std::make_unique<QMimeData>();
    mime->setData(kSymbolMimeType, item->data(0, Qt::UserRole).toString().toUtf8());
    QDrag drag(this);
    drag.setMimeData(mime.release());
    drag.setPixmap(item->icon(0).pixmap(40, 40));
    drag.exec(Qt::CopyAction);
  }
};

} // namespace

LibraryDock::LibraryDock(const SymbolLibrary& library,
                         SymbolRenderCache& render_cache,
                         QWidget* parent)
    : QWidget(parent), library_(library), render_cache_(render_cache) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  search_ = new QLineEdit(this);
  search_->setPlaceholderText(tr("Search symbols..."));
  search_->setClearButtonEnabled(true);
  layout->addWidget(search_);
  tree_ = new SymbolTree(this);
  layout->addWidget(tree_, 1);
  buildTree();
  tree_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  connect(search_, &QLineEdit::textChanged, this, &LibraryDock::filter);
  connect(tree_, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int column) {
    (void)column;
    const QString type = item->data(0, Qt::UserRole).toString();
    if (!type.isEmpty()) {
      emit symbolActivated(type);
    }
  });
}

bool LibraryDock::isPortuguese() const { return QLocale().language() == QLocale::Portuguese; }

void LibraryDock::buildTree() {
  tree_->clear();
  std::map<std::string, std::vector<const SymbolDefinition*>> categories;
  for (const auto& [type, definition] : library_.definitions()) {
    (void)type;
    categories[definition.category].push_back(&definition);
  }
  for (auto& [category, definitions] : categories) {
    std::sort(definitions.begin(),
              definitions.end(),
              [](const SymbolDefinition* left, const SymbolDefinition* right) {
                return left->display_name < right->display_name;
              });
    auto category_item = std::make_unique<QTreeWidgetItem>();
    category_item->setText(0, categoryText(category));
    QFont font = category_item->font(0);
    font.setBold(true);
    category_item->setFont(0, font);
    category_item->setFlags(Qt::ItemIsEnabled);
    QTreeWidgetItem* raw_category = category_item.release();
    tree_->addTopLevelItem(raw_category);

    for (const SymbolDefinition* definition : definitions) {
      auto item = std::make_unique<QTreeWidgetItem>();
      item->setText(0, QString::fromStdString(definition->displayName(isPortuguese())));
      item->setData(0, Qt::UserRole, QString::fromStdString(definition->type));
      item->setToolTip(0, QString::fromStdString(definition->type));
      if (auto renderer = render_cache_.renderer(definition->type)) {
        item->setIcon(0, QIcon(renderer->icon(QSize(48, 48))));
      }
      raw_category->addChild(item.release());
    }
    raw_category->setExpanded(true);
  }
}

void LibraryDock::filter(const QString& text) {
  const QString needle = text.trimmed();
  for (int category_index = 0; category_index < tree_->topLevelItemCount(); ++category_index) {
    QTreeWidgetItem* category = tree_->topLevelItem(category_index);
    bool any_visible = false;
    for (int child_index = 0; child_index < category->childCount(); ++child_index) {
      QTreeWidgetItem* child = category->child(child_index);
      const QString type = child->data(0, Qt::UserRole).toString();
      const bool matches = needle.isEmpty() ||
                           child->text(0).contains(needle, Qt::CaseInsensitive) ||
                           type.contains(needle, Qt::CaseInsensitive);
      child->setHidden(!matches);
      any_visible = any_visible || matches;
    }
    category->setHidden(!any_visible);
  }
}

} // namespace fiovivo::ui
