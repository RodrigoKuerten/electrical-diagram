#include "library/svg_renderer.hpp"

#include <QFile>
#include <QPainter>
#include <QRectF>
#include <QSize>
#include <QSvgRenderer>

#include <utility>

namespace fiovivo {
namespace {

[[nodiscard]] std::unique_ptr<QSvgRenderer>
loadSvg(const std::filesystem::path& path, const std::map<QString, QString>& color_overrides) {
  QFile file(QString::fromStdString(path.string()));
  if (!file.open(QIODevice::ReadOnly)) {
    return nullptr;
  }
  QByteArray contents = file.readAll();
  for (const auto& [source, replacement] : color_overrides) {
    contents.replace(source.toUtf8(), replacement.toUtf8());
  }
  auto renderer = std::make_unique<QSvgRenderer>(contents);
  if (!renderer->isValid()) {
    return nullptr;
  }
  return renderer;
}

} // namespace

SymbolSvgRenderer::SymbolSvgRenderer(const SymbolDefinition& definition,
                                     std::filesystem::path directory)
    : definition_(definition), directory_(std::move(directory)) {}

SymbolSvgRenderer::~SymbolSvgRenderer() = default;

Result<void> SymbolSvgRenderer::load() { return loadInternal(); }

Result<void> SymbolSvgRenderer::setColorOverrides(const std::map<QString, QString>& overrides) {
  color_overrides_ = overrides;
  return loadInternal();
}

Result<void> SymbolSvgRenderer::loadInternal() {
  const std::filesystem::path base_path = directory_ / (definition_.type + ".svg");
  auto base = loadSvg(base_path, color_overrides_);
  if (!base) {
    return makeError(ErrorCode::IoError, "Could not load symbol SVG", base_path.string());
  }
  std::map<std::string, std::unique_ptr<QSvgRenderer>> states;
  for (const auto& [state, filename] : definition_.states) {
    const std::filesystem::path path = directory_ / filename;
    auto renderer = loadSvg(path, color_overrides_);
    if (!renderer) {
      return makeError(ErrorCode::IoError, "Could not load symbol state SVG", path.string());
    }
    states[state] = std::move(renderer);
  }
  base_ = std::move(base);
  states_ = std::move(states);
  return {};
}

bool SymbolSvgRenderer::valid() const { return base_ != nullptr; }

bool SymbolSvgRenderer::hasState(std::string_view state) const {
  return states_.find(std::string(state)) != states_.end();
}

QSvgRenderer* SymbolSvgRenderer::rendererFor(std::string_view state) {
  if (!state.empty()) {
    const auto iterator = states_.find(std::string(state));
    if (iterator != states_.end()) {
      return iterator->second.get();
    }
  }
  return base_.get();
}

void SymbolSvgRenderer::paint(QPainter& painter, const QRectF& target, const std::string& state) {
  if (QSvgRenderer* renderer = rendererFor(state)) {
    renderer->render(&painter, target);
  }
}

QPixmap SymbolSvgRenderer::icon(QSize size) {
  QPixmap pixmap(size);
  pixmap.fill(Qt::transparent);
  if (pixmap.isNull()) {
    return pixmap;
  }
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  const double margin = 2.0;
  const QRectF target(margin, margin, size.width() - margin * 2.0, size.height() - margin * 2.0);
  if (base_) {
    base_->render(&painter, target);
  }
  return pixmap;
}

SymbolRenderCache::SymbolRenderCache(const SymbolLibrary& library, std::filesystem::path directory)
    : library_(library), directory_(std::move(directory)) {}

Result<void> SymbolRenderCache::loadAll() {
  for (const auto& [type, definition] : library_.definitions()) {
    auto renderer = std::make_shared<SymbolSvgRenderer>(definition, directory_);
    const auto loaded = renderer->load();
    if (!loaded) {
      return loaded.error();
    }
    renderers_[type] = std::move(renderer);
  }
  return {};
}

Result<void> SymbolRenderCache::setColorOverrides(const std::map<QString, QString>& overrides) {
  for (const auto& [type, renderer] : renderers_) {
    (void)type;
    const auto loaded = renderer->setColorOverrides(overrides);
    if (!loaded) {
      return loaded.error();
    }
  }
  return {};
}

std::shared_ptr<SymbolSvgRenderer> SymbolRenderCache::renderer(std::string_view type) {
  const auto iterator = renderers_.find(std::string(type));
  return iterator == renderers_.end() ? nullptr : iterator->second;
}

std::shared_ptr<const SymbolSvgRenderer> SymbolRenderCache::renderer(std::string_view type) const {
  const auto iterator = renderers_.find(std::string(type));
  return iterator == renderers_.end() ? nullptr : iterator->second;
}

} // namespace fiovivo
