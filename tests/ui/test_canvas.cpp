#include "core/model.hpp"
#include "io/json_io.hpp"
#include "library/svg_renderer.hpp"
#include "library/symbol_definition.hpp"
#include "ui/app_settings.hpp"
#include "ui/component_item.hpp"
#include "ui/document.hpp"
#include "ui/export_utils.hpp"
#include "ui/main_window.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/sheet_view.hpp"
#include "ui/simulation_controller.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <QTranslator>

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>

#include "unit/core/test_helpers.hpp"

#ifndef FIOVIVO_ASSET_DIR
  #define FIOVIVO_ASSET_DIR "assets"
#endif

namespace fiovivo::ui {
namespace {

[[nodiscard]] QString assetRoot() {
  const QString installed = QCoreApplication::applicationDirPath() + "/assets";
  if (QFileInfo::exists(installed + "/symbols/contactor_coil.json")) {
    return installed;
  }
  return QString::fromUtf8(FIOVIVO_ASSET_DIR);
}

class CanvasTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() { qputenv("QT_QPA_PLATFORM", "offscreen"); }

  void SetUp() override {
    ASSERT_TRUE(library_.loadFromDirectory((assetRoot() + "/symbols").toStdString()));
    cache_ =
        std::make_unique<SymbolRenderCache>(library_, (assetRoot() + "/symbols").toStdString());
    ASSERT_TRUE(cache_->loadAll());
    document_ = std::make_unique<Document>();
    scene_ = std::make_unique<SheetScene>(document_.get(), &library_, cache_.get());
  }

  void TearDown() override {
    scene_.reset();
    document_.reset();
    cache_.reset();
  }

  SymbolLibrary library_;
  std::unique_ptr<SymbolRenderCache> cache_;
  std::unique_ptr<Document> document_;
  std::unique_ptr<SheetScene> scene_;
};

TEST(SymbolRenderTest, AllSymbolRenderersLoad) {
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory((assetRoot() + "/symbols").toStdString()));
  SymbolRenderCache cache(library, (assetRoot() + "/symbols").toStdString());
  const auto loaded = cache.loadAll();
  ASSERT_TRUE(loaded) << loaded.error().toString();
}

TEST_F(CanvasTest, InsertsCadeSimuStyleMultiPoleAndLogicSymbols) {
  scene_->addSymbolAt("contactor_3p", {40.0, 40.0});
  scene_->addSymbolAt("logic_and", {140.0, 40.0});
  scene_->addSymbolAt("sensor_inductive_no", {240.0, 40.0});
  ASSERT_EQ(document_->diagram().componentCount(), 3U);
  const Component& contactor = document_->currentSheet()->components[0];
  // CADe_SIMU's "Contactor III" is a two-pole power contact block (not three poles).
  EXPECT_EQ(contactor.pins.size(), 4U);
  EXPECT_NE(scene_->componentItem(contactor.id), nullptr);
  const Component& gate = document_->currentSheet()->components[1];
  EXPECT_EQ(gate.pins.size(), 3U);
  const Component& sensor = document_->currentSheet()->components[2];
  EXPECT_EQ(sensor.pins.size(), 2U);
}

TEST_F(CanvasTest, InsertsCopyPastesAndUndoesComponents) {
  ASSERT_TRUE(document_->currentSheet() != nullptr);
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {40.0, 40.0});
  EXPECT_EQ(document_->diagram().componentCount(), 1U);
  const ComponentId first = document_->currentSheet()->components.front().id;
  ASSERT_NE(scene_->componentItem(first), nullptr);

  scene_->selectComponent(first);
  scene_->copySelection();
  scene_->pasteClipboard();
  EXPECT_EQ(document_->diagram().componentCount(), 2U);
  EXPECT_EQ(document_->currentSheet()->components[1].tag, "KM2");
  EXPECT_NE(document_->currentSheet()->components[0].tag,
            document_->currentSheet()->components[1].tag);

  document_->undo();
  EXPECT_EQ(document_->diagram().componentCount(), 1U);
  document_->redo();
  EXPECT_EQ(document_->diagram().componentCount(), 2U);
}

TEST_F(CanvasTest, DeleteSelectionRemovesAttachedWires) {
  scene_->addSymbolAt(std::string(component_type::kSupplyAc1), {20.0, 20.0});
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {150.0, 20.0});
  ASSERT_EQ(document_->diagram().componentCount(), 2U);
  const ComponentId supply = document_->currentSheet()->components[0].id;
  const ComponentId coil = document_->currentSheet()->components[1].id;
  const auto wire =
      test::connect(document_->diagram(), document_->currentSheetId(), supply, "L", coil, "A1");
  ASSERT_TRUE(wire.valid());
  document_->setDiagram(document_->diagram(), true);
  scene_->setSheet(document_->currentSheetId());
  EXPECT_EQ(document_->diagram().wireCount(), 1U);

  scene_->selectComponent(coil);
  scene_->deleteSelection();
  EXPECT_EQ(document_->diagram().componentCount(), 1U);
  EXPECT_EQ(document_->diagram().wireCount(), 0U);
}

TEST_F(CanvasTest, SimulationFrameUpdatesVisualState) {
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {40.0, 40.0});
  const ComponentId id = document_->currentSheet()->components.front().id;
  ComponentItem* item = scene_->componentItem(id);
  ASSERT_NE(item, nullptr);
  SimFrame frame;
  frame.time_ms = 10;
  frame.cycle = 1;
  SimItemState state;
  state.energized = true;
  frame.states.insert(id.value(), state);
  scene_->setSimulationInteractive(true);
  scene_->setSimulationFrame(frame);
  EXPECT_TRUE(item->hasSimulationState());
  EXPECT_TRUE(item->simulationState().energized);
  EXPECT_EQ(item->symbolState(), "active");
  scene_->clearSimulation();
  EXPECT_FALSE(item->hasSimulationState());
}

TEST_F(CanvasTest, SvgPngAndPdfExportsAreCreated) {
  scene_->addSymbolAt(std::string(component_type::kSupplyAc1), {20.0, 20.0});
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {150.0, 20.0});
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  const QString svg = directory.filePath("canvas.svg");
  const QString svg2 = directory.filePath("canvas2.svg");
  const QString png = directory.filePath("canvas.png");
  const QString pdf = directory.filePath("canvas.pdf");
  QString error;
  ASSERT_TRUE(exportSvg(*scene_, svg, false, &error)) << error.toStdString();
  ASSERT_TRUE(exportSvg(*scene_, svg2, false, &error)) << error.toStdString();
  ASSERT_TRUE(exportPng(*scene_, png, PngExportOptions{1, false}, &error)) << error.toStdString();
  ASSERT_TRUE(exportPdf(*scene_, pdf, &error)) << error.toStdString();

  QFile svg_file(svg);
  ASSERT_TRUE(svg_file.open(QIODevice::ReadOnly));
  const QByteArray svg_data = svg_file.readAll();
  EXPECT_TRUE(svg_data.contains("<svg"));
  EXPECT_TRUE(svg_data.contains("<text"));
  EXPECT_FALSE(svg_data.contains("<image"));
  QFile svg_file2(svg2);
  ASSERT_TRUE(svg_file2.open(QIODevice::ReadOnly));
  EXPECT_EQ(svg_data, svg_file2.readAll()) << "SVG export must be deterministic";
  EXPECT_GT(QFileInfo(png).size(), 0);
  EXPECT_GT(QFileInfo(pdf).size(), 0);
}

TEST_F(CanvasTest, DragBetweenPinsCreatesAWire) {
  SheetView view;
  view.resize(1000, 760);
  view.setSheetScene(scene_.get());
  view.show();
  QTest::qWait(20);
  scene_->addSymbolAt(std::string(component_type::kSupplyAc1), {20.0, 20.0});
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {180.0, 20.0});
  ASSERT_EQ(document_->diagram().componentCount(), 2U);
  const ComponentId supply = document_->currentSheet()->components[0].id;
  const ComponentId coil = document_->currentSheet()->components[1].id;
  ComponentItem* supply_item = scene_->componentItem(supply);
  ComponentItem* coil_item = scene_->componentItem(coil);
  ASSERT_NE(supply_item, nullptr);
  ASSERT_NE(coil_item, nullptr);
  const QPoint start =
      view.mapFromScene(supply_item->pinScenePosition(supply_item->component().findPin("L")->id));
  const QPoint end =
      view.mapFromScene(coil_item->pinScenePosition(coil_item->component().findPin("A1")->id));
  QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, start);
  QTest::mouseMove(view.viewport(), end, 20);
  QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, end);
  EXPECT_EQ(document_->diagram().wireCount(), 1U);
}

TEST_F(CanvasTest, FitToPageKeepsThePageInsideTheViewport) {
  document_->currentSheet()->setup.paper = PaperSize::A2;
  scene_->rebuild();
  SheetView view;
  view.resize(780, 540);
  view.setSheetScene(scene_.get());
  view.show();
  QTest::qWait(20);
  view.fitToPage();
  const QRectF page = scene_->pageRect();
  const QRectF visible = view.mapToScene(view.viewport()->rect()).boundingRect();
  EXPECT_TRUE(visible.contains(page.adjusted(-1.0, -1.0, 1.0, 1.0)))
      << "visible=" << visible.left() << "," << visible.top() << " " << visible.width() << "x"
      << visible.height() << " page=" << page.width() << "x" << page.height();
  const QPoint center = view.mapFromScene(page.center());
  EXPECT_NEAR(center.x(), view.viewport()->width() / 2, 3);
  EXPECT_NEAR(center.y(), view.viewport()->height() / 2, 3);
}

TEST_F(CanvasTest, ZoomIsClamped) {
  SheetView view;
  view.setSheetScene(scene_.get());
  view.setZoomPercent(1000);
  EXPECT_EQ(view.zoomPercent(), 400);
  view.setZoomPercent(1);
  EXPECT_EQ(view.zoomPercent(), 25);
}

TEST(TranslationTest, PortugueseCatalogLoads) {
  const QString path = QCoreApplication::applicationDirPath() + "/assets/i18n/fiovivo_pt_BR.qm";
  ASSERT_TRUE(QFileInfo::exists(path)) << path.toStdString();
  QTranslator translator;
  ASSERT_TRUE(translator.load(path));
  ASSERT_TRUE(QCoreApplication::installTranslator(&translator));
  EXPECT_EQ(QCoreApplication::translate("fiovivo::ui::MainWindow", "&File"), "&Arquivo");
  EXPECT_EQ(QCoreApplication::translate("fiovivo::ui::MainWindow", "&Help"), "A&juda");
  QCoreApplication::removeTranslator(&translator);
}

namespace {

[[nodiscard]] bool waitFor(const std::function<bool()>& predicate, int timeout_ms) {
  QElapsedTimer timer;
  timer.start();
  while (!predicate() && timer.elapsed() < timeout_ms) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    QThread::msleep(1);
  }
  return predicate();
}

} // namespace

TEST(SimulationControllerTest, RunsAndReactsToInputsInWorkerThread) {
  const QString example = QCoreApplication::applicationDirPath() + "/examples/partida-direta.dcj";
  if (!QFileInfo::exists(example)) {
    GTEST_SKIP() << "Bundled example not copied next to the test executable";
  }
  const auto loaded = fiovivo::io::loadFile(example.toStdString());
  ASSERT_TRUE(loaded) << loaded.error().toString();
  fiovivo::Diagram diagram = loaded.value();
  const fiovivo::Component* start_component = diagram.findComponentByTag("S1");
  const fiovivo::Component* coil_component = diagram.findComponentByTag("KM1");
  ASSERT_NE(start_component, nullptr);
  ASSERT_NE(coil_component, nullptr);
  const fiovivo::ComponentId start = start_component->id;
  const fiovivo::ComponentId coil = coil_component->id;

  SimulationController controller;
  QSignalSpy frames(&controller, &SimulationController::frameReady);
  QSignalSpy states(&controller, &SimulationController::stateChanged);
  controller.loadDiagram(diagram, 10, 1.0);
  controller.start();
  ASSERT_TRUE(waitFor([&] { return frames.count() > 0; }, 2000));
  EXPECT_EQ(controller.state(), SimulationController::State::Running);

  controller.setInput(start.value(), true);
  const bool coil_energized = waitFor(
      [&] {
        if (frames.isEmpty()) {
          return false;
        }
        const SimFrame frame = frames.last().at(0).value<SimFrame>();
        const auto iterator = frame.states.find(coil.value());
        return iterator != frame.states.end() && iterator.value().energized;
      },
      2000);
  EXPECT_TRUE(coil_energized);
  EXPECT_TRUE(states.count() > 0);

  controller.pause();
  EXPECT_TRUE(
      waitFor([&] { return controller.state() == SimulationController::State::Paused; }, 2000));
  controller.resetSimulation();
  EXPECT_TRUE(
      waitFor([&] { return controller.state() == SimulationController::State::Stopped; }, 2000));
}

TEST(CanvasRenderingPerformanceTest, RendersFiveHundredComponents) {
  const QString root = assetRoot();
  SymbolLibrary library;
  ASSERT_TRUE(library.loadFromDirectory((root + "/symbols").toStdString()));
  SymbolRenderCache cache(library, (root + "/symbols").toStdString());
  ASSERT_TRUE(cache.loadAll());
  const SymbolDefinition* definition = library.find(component_type::kContactorCoil);
  ASSERT_NE(definition, nullptr);

  Diagram diagram;
  Sheet sheet;
  sheet.id = diagram.ids().newSheetId();
  sheet.name = "Performance";
  sheet.setup.paper = PaperSize::A1;
  sheet.setup.orientation = Orientation::Landscape;
  const SheetId sheet_id = diagram.addSheet(std::move(sheet)).value();
  for (int index = 0; index < 500; ++index) {
    const double x = 20.0 + static_cast<double>(index % 20) * 55.0;
    const double y = 20.0 + static_cast<double>(index / 20) * 30.0;
    Component component =
        instantiateSymbol(*definition, diagram, {x, y}, "KM" + std::to_string(index + 1));
    ASSERT_TRUE(diagram.addComponent(sheet_id, std::move(component)));
  }
  Document document;
  const auto begin = std::chrono::steady_clock::now();
  document.setDiagram(std::move(diagram), true);
  SheetScene scene(&document, &library, &cache);
  const QRectF page = scene.pageRect();
  QImage image(static_cast<int>(page.width() * 2.0),
               static_cast<int>(page.height() * 2.0),
               QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::white);
  QPainter painter(&image);
  scene.render(&painter, QRectF(QPointF(0.0, 0.0), QSizeF(image.size())), page);
  painter.end();
  const auto end = std::chrono::steady_clock::now();
  const double elapsed_ms = std::chrono::duration<double, std::milli>(end - begin).count();
  std::cout << "[PERFORMANCE] 500-component sheet open+render: " << elapsed_ms << " ms\n";
#ifdef NDEBUG
  // Exact CADe_SIMU artwork has more primitives than the old hand-drawn placeholders.
  constexpr double kBudgetMs = 500.0;
#else
  constexpr double kBudgetMs = 1500.0;
#endif
  EXPECT_LT(elapsed_ms, kBudgetMs);
}

TEST(MainWindowSmokeTest, OpensBundledExample) {
  AppSettings settings;
  MainWindow window(assetRoot(), settings);
  ASSERT_TRUE(window.isValid());
  const QString example = QCoreApplication::applicationDirPath() + "/examples/partida-direta.dcj";
  if (!QFileInfo::exists(example)) {
    GTEST_SKIP() << "Bundled example not copied next to the test executable";
  }
  ASSERT_TRUE(window.openDocument(example));
  window.show();
  QTest::qWait(20);
  EXPECT_NE(window.findChild<SheetView*>(), nullptr);
  EXPECT_NE(window.findChild<SheetScene*>(), nullptr);
}

TEST(MainWindowSmokeTest, WorkspaceActionsResizeTheScene) {
  AppSettings settings;
  MainWindow window(assetRoot(), settings);
  ASSERT_TRUE(window.isValid());
  window.show();
  QTest::qWait(20);
  auto* scene = window.findChild<SheetScene*>();
  ASSERT_NE(scene, nullptr);
  QAction* wider = nullptr;
  QAction* narrower = nullptr;
  for (QAction* action : window.findChildren<QAction*>()) {
    if (action->text().contains("Increase Canvas", Qt::CaseInsensitive) &&
        action->text().contains("Width", Qt::CaseInsensitive)) {
      wider = action;
    }
    if (action->text().contains("Decrease Canvas", Qt::CaseInsensitive) &&
        action->text().contains("idth", Qt::CaseInsensitive)) {
      narrower = action;
    }
  }
  ASSERT_NE(wider, nullptr);
  ASSERT_NE(narrower, nullptr);
  const double initial = scene->workspaceRect().width();
  wider->trigger();
  QTest::qWait(20);
  const Vec2 size = scene->document()->currentSheet()->setup.workspace_size;
  EXPECT_GT(size.x, 0.0);
  EXPECT_GT(scene->workspaceRect().width(), initial);
  narrower->trigger();
  QTest::qWait(20);
  EXPECT_DOUBLE_EQ(scene->workspaceRect().width(), initial);
  scene->document()->markClean();
  window.close();
}

TEST(MainWindowSmokeTest, StartsPausesAndResetsSimulation) {
  AppSettings settings;
  MainWindow window(assetRoot(), settings);
  ASSERT_TRUE(window.isValid());
  const QString example = QCoreApplication::applicationDirPath() + "/examples/partida-direta.dcj";
  if (!QFileInfo::exists(example)) {
    GTEST_SKIP() << "Bundled example not copied next to the test executable";
  }
  ASSERT_TRUE(window.openDocument(example));
  window.show();
  QTest::qWait(20);
  QAction* run = nullptr;
  QAction* reset = nullptr;
  for (QAction* action : window.findChildren<QAction*>()) {
    if (action->shortcut() == QKeySequence(Qt::Key_F5)) {
      run = action;
    }
    if (action->shortcut() == QKeySequence(Qt::SHIFT | Qt::Key_F5)) {
      reset = action;
    }
  }
  ASSERT_NE(run, nullptr);
  ASSERT_NE(reset, nullptr);
  EXPECT_TRUE(run->text().contains("Start", Qt::CaseInsensitive));
  run->trigger();
  ASSERT_TRUE(waitFor([&] { return run->text().contains("Pause", Qt::CaseInsensitive); }, 3000));
  run->trigger();
  ASSERT_TRUE(waitFor([&] { return run->text().contains("Resume", Qt::CaseInsensitive); }, 3000));
  reset->trigger();
  ASSERT_TRUE(waitFor([&] { return run->text().contains("Start", Qt::CaseInsensitive); }, 3000));
  window.close();
}

} // namespace

TEST_F(CanvasTest, WorkspaceSizeExtendsSceneWithoutMovingComponents) {
  scene_->addSymbolAt(std::string(component_type::kContactorCoil), {80.0, 70.0});
  ASSERT_EQ(document_->diagram().componentCount(), 1U);
  const Vec2 before = document_->currentSheet()->components.front().position;
  const SheetId sheet_id = document_->currentSheetId();
  document_->mutate(QStringLiteral("Resize workspace"), [sheet_id](Diagram& diagram) {
    Sheet* sheet = diagram.findSheet(sheet_id);
    ASSERT_NE(sheet, nullptr);
    sheet->setup.workspace_size = {800.0, 600.0};
  });
  const QRectF workspace = scene_->workspaceRect();
  EXPECT_DOUBLE_EQ(workspace.width(), 800.0);
  EXPECT_DOUBLE_EQ(workspace.height(), 600.0);
  EXPECT_TRUE(scene_->sceneRect().contains(QRectF(0.0, 0.0, 800.0, 600.0)));
  const Vec2 after = document_->currentSheet()->components.front().position;
  EXPECT_EQ(before, after);
}

} // namespace fiovivo::ui


int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("FioVivoTest"));
  QCoreApplication::setApplicationName(QStringLiteral("FioVivoTest"));
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
