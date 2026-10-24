#include "core/model.hpp"
#include "library/svg_renderer.hpp"
#include "library/symbol_definition.hpp"
#include "ui/app_settings.hpp"
#include "ui/dialogs/preferences_dialog.hpp"
#include "ui/document.hpp"
#include "ui/main_window.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/theme.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QKeySequence>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QTimer>

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "unit/core/test_helpers.hpp"

#ifndef FIOVIVO_ASSET_DIR
  #define FIOVIVO_ASSET_DIR "assets"
#endif

#ifndef FIOVIVO_GOLDEN_DIR
  #define FIOVIVO_GOLDEN_DIR "tests/golden"
#endif

#ifndef FIOVIVO_SOURCE_DIR
  #define FIOVIVO_SOURCE_DIR "."
#endif

namespace fiovivo::ui {
namespace {

bool g_update_golden = false;

constexpr double kGoldenPixelTolerance = 0.01; // at most 1% of pixels may differ
constexpr int kGoldenChannelTolerance = 12;    // per-channel difference counted as a mismatch

[[nodiscard]] QString assetRoot() {
  const QString installed = QCoreApplication::applicationDirPath() + "/assets";
  if (QFileInfo::exists(installed + "/symbols/contactor_coil.json")) {
    return installed;
  }
  return QString::fromUtf8(FIOVIVO_ASSET_DIR);
}

[[nodiscard]] QString goldenDirectory() {
  QDir directory(QString::fromUtf8(FIOVIVO_GOLDEN_DIR));
  return directory.filePath(QStringLiteral("theme"));
}

[[nodiscard]] QImage
renderScene(SheetScene& scene, SymbolRenderCache& cache, const ThemeMode mode) {
  const ThemePalette palette = applyTheme(*qApp, mode);
  const auto colors = cache.setColorOverrides(symbolColorOverrides(palette));
  if (!colors) {
    ADD_FAILURE() << "Could not apply symbol colours: " << colors.error().toString();
    return {};
  }
  scene.setThemePalette(palette);
  scene.setGridVisible(true);
  scene.setGridSize(5.0);
  scene.update();
  QTest::qWait(1);

  const QRectF page = scene.pageRect();
  constexpr double kScale = 2.0;
  constexpr double kPixelsPerMillimetre = 3.7795275591;
  const QSize size(static_cast<int>(std::lround(page.width() * kPixelsPerMillimetre * kScale)),
                   static_cast<int>(std::lround(page.height() * kPixelsPerMillimetre * kScale)));
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(palette.color(ThemeToken::SheetBackground));
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  scene.render(&painter, QRectF(QPointF(0.0, 0.0), QSizeF(size)), page);
  painter.end();
  return image;
}

struct ReferenceDiagram {
  Document document;
  std::unique_ptr<SymbolLibrary> library;
  std::unique_ptr<SymbolRenderCache> cache;
  std::unique_ptr<SheetScene> scene;
  ComponentId coil_id;
};

[[nodiscard]] std::unique_ptr<ReferenceDiagram> makeReferenceDiagram() {
  auto reference = std::make_unique<ReferenceDiagram>();
  reference->library = std::make_unique<SymbolLibrary>();
  if (!reference->library->loadFromDirectory((assetRoot() + "/symbols").toStdString())) {
    return nullptr;
  }
  reference->cache = std::make_unique<SymbolRenderCache>(*reference->library,
                                                         (assetRoot() + "/symbols").toStdString());
  if (!reference->cache->loadAll()) {
    return nullptr;
  }

  Diagram diagram;
  diagram.metadata.title = "Theme reference";
  diagram.metadata.author = "FioVivo";
  diagram.metadata.created_utc = "2026-10-24T00:00:00Z";
  diagram.metadata.modified_utc = "2026-10-24T00:00:00Z";
  diagram.metadata.revision = "A";

  Sheet sheet;
  sheet.id = diagram.ids().newSheetId();
  sheet.name = "Theme";
  sheet.setup.paper = PaperSize::A4;
  sheet.setup.orientation = Orientation::Landscape;
  sheet.setup.margin = 10.0;
  sheet.setup.show_frame = true;
  const SheetId sheet_id = diagram.addSheet(std::move(sheet)).value();

  const SymbolDefinition* supply = reference->library->find(component_type::kSupplyAc1);
  const SymbolDefinition* coil = reference->library->find(component_type::kContactorCoil);
  const SymbolDefinition* motor = reference->library->find(component_type::kMotor3ph);
  if (supply == nullptr || coil == nullptr || motor == nullptr) {
    return nullptr;
  }

  const ComponentId supply_id =
      diagram.addComponent(sheet_id, instantiateSymbol(*supply, diagram, {30.0, 40.0}, "X1"))
          .value();
  reference->coil_id =
      diagram.addComponent(sheet_id, instantiateSymbol(*coil, diagram, {125.0, 45.0}, "KM1"))
          .value();
  (void)diagram.addComponent(sheet_id, instantiateSymbol(*motor, diagram, {220.0, 45.0}, "M1"));

  const WireId wire = test::connect(diagram, sheet_id, supply_id, "L", reference->coil_id, "A1");
  if (!wire.valid()) {
    return nullptr;
  }
  reference->document.setDiagram(std::move(diagram), true);
  reference->scene = std::make_unique<SheetScene>(
      &reference->document, reference->library.get(), reference->cache.get());
  reference->scene->selectComponent(reference->coil_id);
  return reference;
}

[[nodiscard]] std::size_t differingPixels(const QImage& first, const QImage& second) {
  if (first.size() != second.size()) {
    return static_cast<std::size_t>(first.width()) * static_cast<std::size_t>(first.height());
  }
  const QImage left = first.convertToFormat(QImage::Format_ARGB32);
  const QImage right = second.convertToFormat(QImage::Format_ARGB32);
  std::size_t different = 0;
  for (int y = 0; y < left.height(); ++y) {
    const auto* left_line = reinterpret_cast<const QRgb*>(left.constScanLine(y));
    const auto* right_line = reinterpret_cast<const QRgb*>(right.constScanLine(y));
    for (int x = 0; x < left.width(); ++x) {
      const QRgb a = left_line[x];
      const QRgb b = right_line[x];
      if (std::abs(qRed(a) - qRed(b)) > kGoldenChannelTolerance ||
          std::abs(qGreen(a) - qGreen(b)) > kGoldenChannelTolerance ||
          std::abs(qBlue(a) - qBlue(b)) > kGoldenChannelTolerance ||
          std::abs(qAlpha(a) - qAlpha(b)) > kGoldenChannelTolerance) {
        ++different;
      }
    }
  }
  return different;
}

[[nodiscard]] bool containsHexColor(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  const QString contents = QString::fromUtf8(file.readAll());
  static const QRegularExpression expression(
      QStringLiteral(R"(#(?:[0-9a-fA-F]{3}|[0-9a-fA-F]{4}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})\b)"));
  return expression.match(contents).hasMatch();
}

} // namespace

TEST(ThemePaletteTest, EveryTokenExistsInBothThemes) {
  const auto& tokens = allThemeTokens();
  ASSERT_EQ(tokens.size(), kThemeTokenCount);
  QSet<QString> names;
  for (const ThemeToken token : tokens) {
    if (token == ThemeToken::Count) {
      continue;
    }
    const QString name = QString::fromStdString(std::string(themeTokenName(token)));
    EXPECT_FALSE(name.isEmpty());
    EXPECT_FALSE(names.contains(name)) << "duplicate token name: " << name.toStdString();
    names.insert(name);
    for (const ThemeMode mode : {ThemeMode::Light, ThemeMode::Dark}) {
      const QColor color = themePalette(mode).color(token);
      EXPECT_TRUE(color.isValid()) << "invalid colour for " << name.toStdString() << " in mode "
                                   << themeModeName(mode).toStdString();
      EXPECT_GT(color.alpha(), 0);
    }
  }
}

TEST(ThemePaletteTest, NoHexColorsOutsidePaletteModule) {
  const QString root = QString::fromUtf8(FIOVIVO_SOURCE_DIR);
  const QStringList directories = {root + "/src", root + "/tools", root + "/tests"};
  QStringList violations;
  for (const QString& directory : directories) {
    QDirIterator iterator(directory,
                          {QStringLiteral("*.cpp"), QStringLiteral("*.hpp"), QStringLiteral("*.h")},
                          QDir::Files,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
      const QString path = iterator.next();
      if (path.endsWith(QStringLiteral("src/ui/theme.cpp"))) {
        continue;
      }
      if (containsHexColor(path)) {
        violations.push_back(path);
      }
    }
  }
  EXPECT_TRUE(violations.isEmpty()) << "Hex colours outside src/ui/theme.cpp:\n"
                                    << violations.join('\n').toStdString();
}

TEST(ThemePaletteTest, ContrastPairsMeetWcagAa) {
  for (const ThemeMode mode : {ThemeMode::Light, ThemeMode::Dark}) {
    const ThemePalette palette = themePalette(mode);
    for (const auto& [foreground, background] : contrastPairs()) {
      const double ratio = contrastRatio(palette.color(foreground), palette.color(background));
      EXPECT_GE(ratio, 4.5) << themeModeName(mode).toStdString() << ": "
                            << std::string(themeTokenName(foreground)) << " on "
                            << std::string(themeTokenName(background)) << " = " << ratio;
    }
  }
}

TEST(ThemeRuntimeTest, LightDarkLightRestoresTheCanvasExactly) {
  auto reference = makeReferenceDiagram();
  ASSERT_NE(reference, nullptr);
  ASSERT_NE(reference->scene, nullptr);

  const QImage light = renderScene(*reference->scene, *reference->cache, ThemeMode::Light);
  const QImage dark = renderScene(*reference->scene, *reference->cache, ThemeMode::Dark);
  const QImage light_again = renderScene(*reference->scene, *reference->cache, ThemeMode::Light);
  ASSERT_FALSE(light.isNull());
  ASSERT_FALSE(dark.isNull());
  ASSERT_FALSE(light_again.isNull());
  EXPECT_EQ(light.size(), dark.size());
  EXPECT_GT(differingPixels(light, dark), 0U);
  EXPECT_EQ(differingPixels(light, light_again), 0U)
      << "Switching light -> dark -> light must restore the exact same pixels";
  EXPECT_EQ(QApplication::palette().color(QPalette::Window),
            themePalette(ThemeMode::Light).color(ThemeToken::WindowBackground));
}

TEST(ThemeRuntimeTest, PreferencesSwitchTheWholeWindowWithoutRestart) {
  AppSettings settings;
  settings.setTheme(QStringLiteral("light"));
  settings.applyAppearance(*qApp);

  MainWindow window(assetRoot(), settings);
  ASSERT_TRUE(window.isValid());
  window.show();
  QTest::qWait(20);

  QAction* preferences = nullptr;
  for (QAction* action : window.findChildren<QAction*>()) {
    if (action->shortcut() == QKeySequence(Qt::CTRL | Qt::Key_Comma)) {
      preferences = action;
      break;
    }
  }
  ASSERT_NE(preferences, nullptr);

  const auto choose_theme = [&](const QString& mode) {
    bool handled = false;
    QTimer::singleShot(0, [&handled, mode] {
      auto* dialog = qobject_cast<PreferencesDialog*>(QApplication::activeModalWidget());
      if (dialog == nullptr) {
        return;
      }
      QComboBox* theme_combo = nullptr;
      for (QComboBox* combo : dialog->findChildren<QComboBox*>()) {
        if (combo->findData(mode) >= 0) {
          theme_combo = combo;
          break;
        }
      }
      if (theme_combo != nullptr) {
        theme_combo->setCurrentIndex(theme_combo->findData(mode));
      }
      auto* buttons = dialog->findChild<QDialogButtonBox*>();
      if (buttons != nullptr) {
        buttons->button(QDialogButtonBox::Ok)->click();
        handled = theme_combo != nullptr;
      }
    });
    preferences->trigger();
    return handled;
  };

  EXPECT_TRUE(choose_theme(QStringLiteral("dark")));
  EXPECT_EQ(settings.theme(), QStringLiteral("dark"));
  EXPECT_EQ(QApplication::palette().color(QPalette::Window),
            darkTheme().color(ThemeToken::WindowBackground));
  auto* scene = window.findChild<SheetScene*>();
  ASSERT_NE(scene, nullptr);
  EXPECT_EQ(scene->themePalette().mode, ThemeMode::Dark);

  EXPECT_TRUE(choose_theme(QStringLiteral("light")));
  EXPECT_EQ(settings.theme(), QStringLiteral("light"));
  EXPECT_EQ(QApplication::palette().color(QPalette::Window),
            lightTheme().color(ThemeToken::WindowBackground));
  EXPECT_EQ(scene->themePalette().mode, ThemeMode::Light);
  window.close();
}

TEST(ThemeGoldenTest, LightAndDarkReferenceScenes) {
  auto reference = makeReferenceDiagram();
  ASSERT_NE(reference, nullptr);
  ASSERT_NE(reference->scene, nullptr);

  const QString directory = goldenDirectory();
  if (!QDir().mkpath(directory)) {
    FAIL() << "Could not create golden directory " << directory.toStdString();
  }

  for (const ThemeMode mode : {ThemeMode::Light, ThemeMode::Dark}) {
    const QString name = themeModeName(mode) + QStringLiteral(".png");
    const QString path = QDir(directory).filePath(name);
    const QImage image = renderScene(*reference->scene, *reference->cache, mode);
    ASSERT_FALSE(image.isNull());
    if (g_update_golden) {
      ASSERT_TRUE(image.save(path, "PNG")) << "Could not save " << path.toStdString();
      std::printf("[GOLDEN] updated tests/golden/theme/%s\n", name.toUtf8().constData());
      continue;
    }
    ASSERT_TRUE(QFileInfo::exists(path))
        << "Missing golden " << path.toStdString() << "; run with --update-golden";
    const QImage expected(path);
    ASSERT_FALSE(expected.isNull());
    const std::size_t total =
        static_cast<std::size_t>(image.width()) * static_cast<std::size_t>(image.height());
    const std::size_t different = differingPixels(image, expected);
    const double ratio = total == 0 ? 0.0
                                    : static_cast<double>(different) / static_cast<double>(total);
    EXPECT_LE(ratio, kGoldenPixelTolerance)
        << "Theme golden " << name.toStdString() << " differs in " << different << " of " << total
        << " pixels (" << ratio * 100.0 << "%, tolerance " << kGoldenPixelTolerance * 100.0
        << "%, channel tolerance " << kGoldenChannelTolerance << ")";
  }
}

} // namespace fiovivo::ui

int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("FioVivoThemeTests"));
  QCoreApplication::setApplicationName(QStringLiteral("FioVivoThemeTests"));

  std::vector<char*> arguments;
  arguments.reserve(static_cast<std::size_t>(argc));
  for (int index = 0; index < argc; ++index) {
    if (std::strcmp(argv[index], "--update-golden") == 0) {
      fiovivo::ui::g_update_golden = true;
      continue;
    }
    arguments.push_back(argv[index]);
  }
  int filtered_argc = static_cast<int>(arguments.size());
  arguments.push_back(nullptr);

  testing::InitGoogleTest(&filtered_argc, arguments.data());
  return RUN_ALL_TESTS();
}
