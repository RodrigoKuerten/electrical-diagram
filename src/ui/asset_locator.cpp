#include "ui/asset_locator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#ifndef FIOVIVO_SOURCE_ASSET_DIR
  #define FIOVIVO_SOURCE_ASSET_DIR ""
#endif

namespace fiovivo::ui {
namespace {

[[nodiscard]] bool isAssetRoot(const QDir& directory) {
  return QFileInfo::exists(directory.filePath("symbols/contactor_coil.json"));
}

[[nodiscard]] QStringList candidateRoots() {
  QStringList candidates;
  const QString environment = qEnvironmentVariable("FIOVIVO_ASSET_DIR");
  if (!environment.isEmpty()) {
    candidates << environment;
  }
  const QString application = QCoreApplication::applicationDirPath();
  candidates << application + "/assets";
  candidates << application + "/../share/fiovivo/assets";
  candidates << application + "/../assets";
  const QString source = QString::fromUtf8(FIOVIVO_SOURCE_ASSET_DIR);
  if (!source.isEmpty()) {
    candidates << source;
  }
  return candidates;
}

} // namespace

QString findAssetRoot() {
  for (const QString& candidate : candidateRoots()) {
    const QDir directory(candidate);
    if (isAssetRoot(directory)) {
      return directory.absolutePath();
    }
  }
  return {};
}

QString findExamplesDirectory() {
  const QString root = findAssetRoot();
  if (root.isEmpty()) {
    return {};
  }
  const QDir assets(root);
  const QString installed = assets.filePath("examples");
  if (QFileInfo::exists(installed)) {
    return installed;
  }
  const QString sibling = assets.filePath("../examples");
  if (QFileInfo::exists(sibling)) {
    return QDir(sibling).absolutePath();
  }
  return {};
}

QString findTranslationFile(const QString& language_code) {
  const QString filename = QStringLiteral("fiovivo_%1.qm").arg(language_code);
  const QString embedded = QStringLiteral(":/i18n/%1").arg(filename);
  if (QFileInfo::exists(embedded)) {
    return embedded;
  }
  const QString root = findAssetRoot();
  if (!root.isEmpty()) {
    const QString in_assets = QDir(root).filePath("i18n/" + filename);
    if (QFileInfo::exists(in_assets)) {
      return in_assets;
    }
  }
  const QString local = QCoreApplication::applicationDirPath() + "/i18n/" + filename;
  if (QFileInfo::exists(local)) {
    return local;
  }
  return {};
}

QString findSchemaFile() {
  const QString root = findAssetRoot();
  if (!root.isEmpty()) {
    const QString installed = QDir(root).filePath("file-format.schema.json");
    if (QFileInfo::exists(installed)) {
      return installed;
    }
    const QString source = QDir(root).filePath("../docs/file-format.schema.json");
    if (QFileInfo::exists(source)) {
      return QDir(source).absolutePath();
    }
  }
  return {};
}

} // namespace fiovivo::ui
