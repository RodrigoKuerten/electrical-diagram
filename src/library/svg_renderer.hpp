#pragma once

#include "core/result.hpp"
#include "library/symbol_definition.hpp"

#include <QPixmap>
#include <QSize>
#include <QString>

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>

class QPainter;
class QRectF;
class QSvgRenderer;

namespace fiovivo {

// Loads a symbol's base SVG and all predeclared state variants once. Simulation updates select
// one of these already-parsed renderers; SVG is never parsed during a scan cycle.
class SymbolSvgRenderer {
public:
  SymbolSvgRenderer(const SymbolDefinition& definition, std::filesystem::path directory);
  ~SymbolSvgRenderer();

  SymbolSvgRenderer(const SymbolSvgRenderer&) = delete;
  SymbolSvgRenderer& operator=(const SymbolSvgRenderer&) = delete;

  Result<void> load();
  // Replaces literal source colours in the SVG text before parsing. The UI supplies the mapping
  // from the theme palette; the library itself does not own theme colours.
  Result<void> setColorOverrides(const std::map<QString, QString>& overrides);
  void paint(QPainter& painter, const QRectF& target, const std::string& state = {});
  [[nodiscard]] QPixmap icon(QSize size);
  [[nodiscard]] const SymbolDefinition& definition() const { return definition_; }
  [[nodiscard]] bool valid() const;
  [[nodiscard]] bool hasState(std::string_view state) const;

private:
  [[nodiscard]] Result<void> loadInternal();
  [[nodiscard]] QSvgRenderer* rendererFor(std::string_view state);

  const SymbolDefinition& definition_;
  std::filesystem::path directory_;
  std::map<QString, QString> color_overrides_;
  std::unique_ptr<QSvgRenderer> base_;
  std::map<std::string, std::unique_ptr<QSvgRenderer>> states_;
};

class SymbolRenderCache {
public:
  SymbolRenderCache(const SymbolLibrary& library, std::filesystem::path directory);

  Result<void> loadAll();
  Result<void> setColorOverrides(const std::map<QString, QString>& overrides);
  [[nodiscard]] std::shared_ptr<SymbolSvgRenderer> renderer(std::string_view type);
  [[nodiscard]] std::shared_ptr<const SymbolSvgRenderer> renderer(std::string_view type) const;
  [[nodiscard]] const SymbolLibrary& library() const { return library_; }

private:
  const SymbolLibrary& library_;
  std::filesystem::path directory_;
  std::map<std::string, std::shared_ptr<SymbolSvgRenderer>> renderers_;
};

} // namespace fiovivo
