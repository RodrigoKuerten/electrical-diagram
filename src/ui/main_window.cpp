#include "ui/main_window.hpp"

#include "core/types.hpp"
#include "io/bom.hpp"
#include "io/json_io.hpp"
#include "library/svg_renderer.hpp"
#include "library/symbol_definition.hpp"
#include "ui/app_settings.hpp"
#include "ui/asset_locator.hpp"
#include "ui/component_factory.hpp"
#include "ui/dialogs/about_dialog.hpp"
#include "ui/dialogs/new_project_dialog.hpp"
#include "ui/dialogs/preferences_dialog.hpp"
#include "ui/document.hpp"
#include "ui/export_utils.hpp"
#include "ui/library_dock.hpp"
#include "ui/problems_dock.hpp"
#include "ui/properties_dock.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/sheet_view.hpp"
#include "ui/simulation_controller.hpp"
#include "ui/simulation_log_dock.hpp"
#include "ui/ui_helpers.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QIcon>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringConverter>
#include <QStyle>
#include <QTabWidget>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QUndoStack>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <memory>

namespace fiovivo::ui {
namespace {

constexpr int kAutosaveIntervalMs = 2 * 60 * 1000;

[[nodiscard]] QString ensureSuffix(const QString& path, const QString& suffix) {
  if (path.endsWith(suffix, Qt::CaseInsensitive)) {
    return path;
  }
  return path + suffix;
}

class PngOptionsDialog : public QDialog {
public:
  explicit PngOptionsDialog(QWidget* parent) : QDialog(parent) {
    const auto text = [](const char* source) {
      return QCoreApplication::translate("PngOptionsDialog", source);
    };
    setWindowTitle(text("PNG Export Options"));
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();
    scale_ = new QComboBox(this);
    scale_->addItem(text("1x (96 dpi)"), 1);
    scale_->addItem(text("2x (192 dpi)"), 2);
    scale_->addItem(text("4x (384 dpi)"), 4);
    scale_->setCurrentIndex(1);
    transparent_ = new QCheckBox(text("Transparent background"), this);
    form->addRow(text("Scale"), scale_);
    form->addRow(QString(), transparent_);
    layout->addLayout(form);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  }

  [[nodiscard]] PngExportOptions options() const {
    return PngExportOptions{scale_->currentData().toInt(), transparent_->isChecked()};
  }

private:
  QComboBox* scale_ = nullptr;
  QCheckBox* transparent_ = nullptr;
};

} // namespace

MainWindow::MainWindow(QString asset_root, AppSettings& settings, QWidget* parent)
    : QMainWindow(parent), asset_root_(std::move(asset_root)), settings_(settings) {
  examples_directory_ = findExamplesDirectory();
  setWindowTitle(tr("FioVivo"));
  setWindowIcon(QIcon::fromTheme(QStringLiteral("applications-engineering")));
  setDockNestingEnabled(true);
  initializeSymbols();
  if (!initialized_) {
    return;
  }

  document_ = new Document(this);
  scene_ = new SheetScene(document_, library_.get(), render_cache_.get(), this);
  view_ = new SheetView(this);
  view_->setSheetScene(scene_);
  setCentralWidget(view_);
  controller_ = new SimulationController(this);

  createDocks();
  createActions();
  createMenus();
  createToolbar();
  createStatusBar();
  connectSignals();
  applyTheme();

  document_->clear();
  view_->setFocus();

  if (!settings_.windowGeometry().isEmpty()) {
    restoreGeometry(settings_.windowGeometry());
  } else {
    resize(1380, 880);
  }
  if (!settings_.windowState().isEmpty()) {
    restoreState(settings_.windowState());
  }

  setupAutosave();
  QTimer::singleShot(0, this, [this] {
    view_->fitToPage();
    checkRecovery();
  });
  updateWindowTitle();
  runValidation();
  updateRecentMenu();
}

MainWindow::~MainWindow() = default;

void MainWindow::initializeSymbols() {
  library_ = std::make_unique<SymbolLibrary>();
  const QDir symbols(QDir(asset_root_).filePath("symbols"));
  const auto loaded = library_->loadFromDirectory(symbols.absolutePath().toStdString());
  if (!loaded) {
    QMessageBox::critical(this,
                          tr("Symbol library error"),
                          tr("Could not load the symbol library:\n%1")
                              .arg(QString::fromStdString(loaded.error().toString())));
    return;
  }
  render_cache_ =
      std::make_unique<SymbolRenderCache>(*library_, symbols.absolutePath().toStdString());
  const auto rendered = render_cache_->loadAll();
  if (!rendered) {
    QMessageBox::critical(this,
                          tr("Symbol library error"),
                          tr("Could not load symbol artwork:\n%1")
                              .arg(QString::fromStdString(rendered.error().toString())));
    return;
  }
  const QIcon branded(QDir(asset_root_).filePath(QStringLiteral("fiovivo.svg")));
  if (!branded.isNull()) {
    setWindowIcon(branded);
  } else if (const auto renderer = render_cache_->renderer(component_type::kContactorCoil)) {
    setWindowIcon(QIcon(renderer->icon(QSize(64, 64))));
  }
  initialized_ = true;
}

void MainWindow::createActions() {
  auto style_icon = [this](QStyle::StandardPixmap icon) { return style()->standardIcon(icon); };

  new_action_ = new QAction(style_icon(QStyle::SP_FileIcon), tr("&New..."), this);
  new_action_->setShortcut(QKeySequence::New);
  new_action_->setStatusTip(tr("Create a new project"));

  open_action_ = new QAction(style_icon(QStyle::SP_DialogOpenButton), tr("&Open..."), this);
  open_action_->setShortcut(QKeySequence::Open);
  open_action_->setStatusTip(tr("Open a .dcj project"));

  save_action_ = new QAction(style_icon(QStyle::SP_DialogSaveButton), tr("&Save"), this);
  save_action_->setShortcut(QKeySequence::Save);
  save_action_->setStatusTip(tr("Save the current project"));

  save_as_action_ = new QAction(tr("Save &As..."), this);
  save_as_action_->setShortcut(QKeySequence::SaveAs);

  exit_action_ = new QAction(tr("E&xit"), this);
  exit_action_->setShortcut(QKeySequence::Quit);

  undo_action_ = document_->undoStack()->createUndoAction(this, tr("&Undo"));
  undo_action_->setShortcut(QKeySequence::Undo);
  redo_action_ = document_->undoStack()->createRedoAction(this, tr("&Redo"));
  redo_action_->setShortcut(QKeySequence::Redo);

  cut_action_ = new QAction(tr("Cu&t"), this);
  cut_action_->setShortcut(QKeySequence::Cut);
  copy_action_ = new QAction(tr("&Copy"), this);
  copy_action_->setShortcut(QKeySequence::Copy);
  paste_action_ = new QAction(tr("&Paste"), this);
  paste_action_->setShortcut(QKeySequence::Paste);
  duplicate_action_ = new QAction(tr("D&uplicate"), this);
  duplicate_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
  delete_action_ = new QAction(tr("&Delete"), this);
  delete_action_->setShortcut(QKeySequence::Delete);
  select_all_action_ = new QAction(tr("Select &All"), this);
  select_all_action_->setShortcut(QKeySequence::SelectAll);

  zoom_in_action_ = new QAction(tr("Zoom &In"), this);
  zoom_in_action_->setShortcut(QKeySequence::ZoomIn);
  zoom_out_action_ = new QAction(tr("Zoom &Out"), this);
  zoom_out_action_->setShortcut(QKeySequence::ZoomOut);
  fit_action_ = new QAction(tr("&Fit to Page"), this);
  fit_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

  grid_action_ = new QAction(tr("Show &Grid"), this);
  grid_action_->setCheckable(true);
  grid_action_->setChecked(settings_.gridVisible());
  wire_mode_action_ = new QAction(tr("&Wire Mode"), this);
  wire_mode_action_->setCheckable(true);

  run_action_ = new QAction(style_icon(QStyle::SP_MediaPlay), tr("&Start Simulation"), this);
  run_action_->setShortcut(QKeySequence(Qt::Key_F5));
  step_action_ = new QAction(style_icon(QStyle::SP_MediaSkipForward), tr("Step &Cycle"), this);
  step_action_->setShortcut(QKeySequence(Qt::Key_F10));
  step_n_action_ = new QAction(tr("Step &N Cycles..."), this);
  step_n_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F10));
  reset_action_ = new QAction(style_icon(QStyle::SP_MediaStop), tr("&Reset Simulation"), this);
  reset_action_->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
  reset_estop_action_ = new QAction(tr("Reset Emergency Stop(s)"), this);

  validate_action_ = new QAction(tr("&Validate Diagram"), this);
  validate_action_->setShortcut(QKeySequence(Qt::Key_F7));
  preferences_action_ = new QAction(tr("&Preferences..."), this);
  preferences_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
  about_action_ = new QAction(tr("&About FioVivo..."), this);

  connect(new_action_, &QAction::triggered, this, &MainWindow::newProject);
  connect(open_action_, &QAction::triggered, this, &MainWindow::openFileDialog);
  connect(save_action_, &QAction::triggered, this, &MainWindow::saveFile);
  connect(save_as_action_, &QAction::triggered, this, &MainWindow::saveFileAs);
  connect(exit_action_, &QAction::triggered, this, &QWidget::close);
  connect(cut_action_, &QAction::triggered, this, &MainWindow::cutSelection);
  connect(copy_action_, &QAction::triggered, this, &MainWindow::copySelection);
  connect(paste_action_, &QAction::triggered, this, &MainWindow::pasteSelection);
  connect(duplicate_action_, &QAction::triggered, this, &MainWindow::duplicateSelection);
  connect(delete_action_, &QAction::triggered, this, &MainWindow::deleteSelection);
  connect(select_all_action_, &QAction::triggered, this, &MainWindow::selectAll);
  connect(zoom_in_action_, &QAction::triggered, view_, &SheetView::zoomIn);
  connect(zoom_out_action_, &QAction::triggered, view_, &SheetView::zoomOut);
  connect(fit_action_, &QAction::triggered, view_, &SheetView::fitToPage);
  connect(grid_action_, &QAction::toggled, this, [this](bool checked) {
    settings_.setGridVisible(checked);
    scene_->setGridVisible(checked);
  });
  connect(wire_mode_action_, &QAction::toggled, this, [this](bool checked) {
    scene_->setWireMode(checked);
  });
  connect(run_action_, &QAction::triggered, this, &MainWindow::startOrPauseSimulation);
  connect(step_action_, &QAction::triggered, this, &MainWindow::stepSimulation);
  connect(step_n_action_, &QAction::triggered, this, &MainWindow::stepCycles);
  connect(reset_action_, &QAction::triggered, this, &MainWindow::resetSimulation);
  connect(reset_estop_action_, &QAction::triggered, this, &MainWindow::resetEmergencyStops);
  connect(validate_action_, &QAction::triggered, this, &MainWindow::runValidation);
  connect(preferences_action_, &QAction::triggered, this, &MainWindow::showPreferences);
  connect(about_action_, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus() {
  auto* file = menuBar()->addMenu(tr("&File"));
  file->addAction(new_action_);
  file->addAction(open_action_);
  recent_menu_ = file->addMenu(tr("Open &Recent"));
  file->addSeparator();
  file->addAction(save_action_);
  file->addAction(save_as_action_);
  auto* export_menu = file->addMenu(tr("&Export"));
  auto* png = export_menu->addAction(tr("PNG Image..."));
  auto* svg = export_menu->addAction(tr("SVG Vector..."));
  auto* pdf = export_menu->addAction(tr("PDF Document..."));
  auto* bom = export_menu->addAction(tr("Bill of Materials (CSV)..."));
  file->addSeparator();
  auto* print_preview = file->addAction(tr("Print Pre&view..."));
  auto* print = file->addAction(tr("&Print..."));
  print->setShortcut(QKeySequence::Print);
  file->addSeparator();
  file->addAction(exit_action_);
  connect(png, &QAction::triggered, this, &MainWindow::exportPng);
  connect(svg, &QAction::triggered, this, &MainWindow::exportSvg);
  connect(pdf, &QAction::triggered, this, &MainWindow::exportPdf);
  connect(bom, &QAction::triggered, this, &MainWindow::exportBom);
  connect(print_preview, &QAction::triggered, this, &MainWindow::printPreview);
  connect(print, &QAction::triggered, this, &MainWindow::printDocument);

  auto* edit = menuBar()->addMenu(tr("&Edit"));
  edit->addAction(undo_action_);
  edit->addAction(redo_action_);
  edit->addSeparator();
  edit->addAction(cut_action_);
  edit->addAction(copy_action_);
  edit->addAction(paste_action_);
  edit->addAction(duplicate_action_);
  edit->addAction(delete_action_);
  edit->addSeparator();
  edit->addAction(select_all_action_);
  edit->addSeparator();
  edit->addAction(preferences_action_);

  auto* view = menuBar()->addMenu(tr("&View"));
  view->addAction(zoom_in_action_);
  view->addAction(zoom_out_action_);
  view->addAction(fit_action_);
  view->addSeparator();
  view->addAction(grid_action_);
  view->addAction(wire_mode_action_);
  view->addSeparator();

  auto* insert = menuBar()->addMenu(tr("&Insert"));
  auto* note = insert->addAction(tr("Text Note"));
  auto* rectangle = insert->addAction(tr("Rectangle"));
  auto* sheet = insert->addAction(tr("New Sheet"));
  insert->addSeparator();
  auto* symbols = insert->addMenu(tr("Symbol"));
  std::vector<const SymbolDefinition*> definitions;
  for (const auto& [type, definition] : library_->definitions()) {
    (void)type;
    definitions.push_back(&definition);
  }
  std::sort(definitions.begin(),
            definitions.end(),
            [](const SymbolDefinition* left, const SymbolDefinition* right) {
              if (left->category != right->category) {
                return left->category < right->category;
              }
              return left->display_name < right->display_name;
            });
  for (const SymbolDefinition* definition : definitions) {
    QAction* action = symbols->addAction(QString::fromStdString(
        definition->displayName(QLocale().language() == QLocale::Portuguese)));
    const std::string type = definition->type;
    connect(action, &QAction::triggered, this, [this, type] {
      const QPointF position = scene_->snap(view_->mapToScene(view_->viewport()->rect().center()));
      scene_->addSymbolAt(type, position);
    });
  }
  connect(note, &QAction::triggered, this, &MainWindow::insertTextNote);
  connect(rectangle, &QAction::triggered, this, &MainWindow::insertRectangle);
  connect(sheet, &QAction::triggered, this, &MainWindow::addSheet);

  auto* simulate = menuBar()->addMenu(tr("&Simulate"));
  simulate->addAction(run_action_);
  simulate->addAction(step_action_);
  simulate->addAction(step_n_action_);
  simulate->addAction(reset_action_);
  simulate->addSeparator();
  simulate->addAction(reset_estop_action_);
  simulate->addSeparator();
  auto* clear_log = simulate->addAction(tr("Clear Simulation Log"));
  auto* export_log = simulate->addAction(tr("Export Simulation Log..."));
  connect(clear_log, &QAction::triggered, log_dock_, &SimulationLogDock::clear);
  connect(export_log, &QAction::triggered, this, &MainWindow::exportSimulationLog);

  auto* tools = menuBar()->addMenu(tr("&Tools"));
  tools->addAction(validate_action_);
  tools->addSeparator();
  auto* bom_action = tools->addAction(tr("Export Bill of Materials..."));
  connect(bom_action, &QAction::triggered, this, &MainWindow::exportBom);
  auto* output_action = tools->addAction(tr("Show Output"));
  connect(output_action, &QAction::triggered, this, [this] {
    bottom_tabs_->setCurrentWidget(output_);
    bottom_panel_->show();
  });

  auto* help = menuBar()->addMenu(tr("&Help"));
  auto* docs = help->addAction(tr("Documentation"));
  auto* about_qt = help->addAction(tr("About Qt"));
  help->addSeparator();
  help->addAction(about_action_);
  connect(docs, &QAction::triggered, this, &MainWindow::showDocumentation);
  connect(about_qt, &QAction::triggered, qApp, &QApplication::aboutQt);

  view->addAction(library_panel_->toggleViewAction());
  view->addAction(properties_panel_->toggleViewAction());
  view->addAction(bottom_panel_->toggleViewAction());
  updateRecentMenu();
}

void MainWindow::createToolbar() {
  auto* toolbar = addToolBar(tr("Main"));
  toolbar->setObjectName(QStringLiteral("MainToolbar"));
  toolbar->setMovable(false);
  toolbar->addAction(new_action_);
  toolbar->addAction(open_action_);
  toolbar->addAction(save_action_);
  toolbar->addSeparator();
  toolbar->addAction(undo_action_);
  toolbar->addAction(redo_action_);
  toolbar->addSeparator();
  toolbar->addAction(cut_action_);
  toolbar->addAction(copy_action_);
  toolbar->addAction(paste_action_);
  toolbar->addSeparator();
  toolbar->addAction(zoom_in_action_);
  toolbar->addAction(zoom_out_action_);
  toolbar->addAction(fit_action_);
  toolbar->addSeparator();
  toolbar->addAction(run_action_);
  toolbar->addAction(step_action_);
  toolbar->addAction(reset_action_);
  toolbar->addSeparator();
  toolbar->addAction(grid_action_);
  toolbar->addAction(wire_mode_action_);

  addToolBarBreak(Qt::TopToolBarArea);
  auto* context_toolbar = addToolBar(tr("Context"));
  context_toolbar->setObjectName(QStringLiteral("ContextToolbar"));
  context_toolbar->setMovable(false);
  context_toolbar->addWidget(new QLabel(tr("Sheet:"), context_toolbar));
  sheet_combo_ = new QComboBox(context_toolbar);
  sheet_combo_->setMinimumWidth(180);
  context_toolbar->addWidget(sheet_combo_);
  context_toolbar->addSeparator();
  context_toolbar->addWidget(new QLabel(tr("Speed:"), context_toolbar));
  speed_combo_ = new QComboBox(context_toolbar);
  for (double value : {0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0}) {
    speed_combo_->addItem(QString::number(value, 'g', 2) + "x", value);
  }
  const int initial_index = speed_combo_->findData(settings_.simulationSpeed());
  speed_combo_->setCurrentIndex(initial_index >= 0 ? initial_index : 3);
  context_toolbar->addWidget(speed_combo_);

  connect(sheet_combo_, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
    const auto id = sheet_combo_->itemData(index).toULongLong();
    document_->setCurrentSheet(SheetId{id});
    view_->fitToPage();
  });
  connect(speed_combo_, &QComboBox::currentIndexChanged, this, [this](int index) {
    (void)index;
    const double speed = speed_combo_->currentData().toDouble();
    settings_.setSimulationSpeed(speed);
    controller_->setSpeed(speed);
  });
}

void MainWindow::createDocks() {
  library_panel_ = new QDockWidget(tr("Symbol Library"), this);
  library_panel_->setObjectName(QStringLiteral("LibraryPanel"));
  library_dock_ = new LibraryDock(*library_, *render_cache_, library_panel_);
  library_panel_->setWidget(library_dock_);
  addDockWidget(Qt::LeftDockWidgetArea, library_panel_);

  properties_panel_ = new QDockWidget(tr("Properties"), this);
  properties_panel_->setObjectName(QStringLiteral("PropertiesPanel"));
  properties_dock_ = new PropertiesDock(document_, library_.get(), properties_panel_);
  properties_dock_->setScene(scene_);
  properties_panel_->setWidget(properties_dock_);
  addDockWidget(Qt::RightDockWidgetArea, properties_panel_);

  bottom_panel_ = new QDockWidget(tr("Messages"), this);
  bottom_panel_->setObjectName(QStringLiteral("BottomPanel"));
  bottom_tabs_ = new QTabWidget(bottom_panel_);
  problems_dock_ = new ProblemsDock(bottom_tabs_);
  log_dock_ = new SimulationLogDock(bottom_tabs_);
  output_ = new QPlainTextEdit(bottom_tabs_);
  output_->setReadOnly(true);
  output_->setMaximumBlockCount(5000);
  bottom_tabs_->addTab(problems_dock_, tr("Problems"));
  bottom_tabs_->addTab(log_dock_, tr("Simulation Log"));
  bottom_tabs_->addTab(output_, tr("Output"));
  bottom_panel_->setWidget(bottom_tabs_);
  addDockWidget(Qt::BottomDockWidgetArea, bottom_panel_);
  resizeDocks({library_panel_, properties_panel_}, {290, 330}, Qt::Horizontal);
  resizeDocks({bottom_panel_}, {220}, Qt::Vertical);
}

void MainWindow::createStatusBar() {
  zoom_label_ = new QLabel(tr("100%"), this);
  sheet_label_ = new QLabel(tr("Sheet 1"), this);
  cursor_label_ = new QLabel(tr("0.0, 0.0 mm"), this);
  simulation_label_ = new QLabel(tr("Simulation stopped"), this);
  statusBar()->addPermanentWidget(zoom_label_);
  statusBar()->addPermanentWidget(sheet_label_);
  statusBar()->addPermanentWidget(cursor_label_);
  statusBar()->addPermanentWidget(simulation_label_);
  statusBar()->showMessage(tr("Ready"));
}

void MainWindow::connectSignals() {
  connect(document_, &Document::diagramChanged, this, [this] {
    updateWindowTitle();
    updateSheetCombo();
    runValidation();
    updateUndoActions();
    if (controller_->state() != SimulationController::State::Stopped || controller_->isLoaded()) {
      discardSimulation();
    }
  });
  connect(document_, &Document::modifiedChanged, this, [this](bool modified) {
    (void)modified;
    updateWindowTitle();
  });
  connect(document_, &Document::currentSheetChanged, this, [this] {
    updateSheetCombo();
    sheet_label_->setText(scene_->sheetName());
    view_->fitToPage();
  });
  connect(document_->undoStack(), &QUndoStack::indexChanged, this, [this](int index) {
    (void)index;
    updateUndoActions();
  });

  connect(scene_, &SheetScene::cursorMoved, this, [this](const QPointF& position) {
    cursor_label_->setText(
        tr("%1, %2 mm")
            .arg(QString::number(position.x(), 'f', 1), QString::number(position.y(), 'f', 1)));
  });
  connect(scene_, &SheetScene::selectionChanged, this, [this] { updateUndoActions(); });
  connect(scene_, &SheetScene::statusMessage, this, [this](const QString& message) {
    showStatusMessage(message);
    appendOutput(message);
  });
  connect(scene_, &SheetScene::inputRequested, this, &MainWindow::handleInputRequest);
  connect(scene_, &SheetScene::tripRequested, this, &MainWindow::handleTripRequest);
  connect(scene_, &SheetScene::selectorRequested, this, &MainWindow::handleSelectorRequest);
  connect(scene_, &SheetScene::emergencyResetRequested, this, &MainWindow::resetEmergencyStops);
  connect(scene_, &SheetScene::wireModeChanged, this, [this](bool enabled) {
    QSignalBlocker blocker(wire_mode_action_);
    wire_mode_action_->setChecked(enabled);
  });
  connect(scene_, &SheetScene::propertiesRequested, this, [this](quint64 component) {
    scene_->selectComponent(ComponentId{component});
    properties_panel_->show();
    properties_panel_->raise();
  });
  connect(
      scene_, &SheetScene::selectionChanged, this, [this] { properties_dock_->setScene(scene_); });
  connect(view_, &SheetView::zoomChanged, this, [this](int percent) {
    zoom_label_->setText(tr("%1%").arg(percent));
  });
  connect(library_dock_, &LibraryDock::symbolActivated, this, [this](const QString& type) {
    const QPointF position = scene_->snap(view_->mapToScene(view_->viewport()->rect().center()));
    scene_->addSymbolAt(type.toStdString(), position);
  });
  connect(problems_dock_, &ProblemsDock::navigateTo, this, &MainWindow::navigateTo);
  connect(log_dock_, &SimulationLogDock::statusMessage, this, &MainWindow::showStatusMessage);

  connect(controller_, &SimulationController::frameReady, this, &MainWindow::handleSimulationFrame);
  connect(controller_, &SimulationController::logReady, this, [this](const SimLogEntry& entry) {
    log_dock_->appendEntry(entry);
  });
  connect(
      controller_, &SimulationController::stateChanged, this, [this] { handleSimulationState(); });
  connect(
      controller_, &SimulationController::resetOccurred, this, &MainWindow::handleResetOccurred);
  connect(controller_, &SimulationController::faultRaised, this, [this](const QString& message) {
    appendOutput(tr("Simulation error: %1").arg(message));
    showStatusMessage(tr("Simulation stopped: %1").arg(message));
    QMessageBox::warning(this, tr("Simulation error"), message);
  });
  connect(controller_, &SimulationController::simulatedTimeChanged, this, [this](qint64 time) {
    (void)time;
    updateStatusSimulation();
  });
}

void MainWindow::applyTheme() {
  settings_.applyAppearance(*qApp);
  const bool dark = settings_.theme() == "dark";
  scene_->setCanvasBackground(QColor(dark ? "#20262d" : "#d8dee6"));
  scene_->setTextColor(QColor("#111827"));
  scene_->setPinColor(QColor("#111827"));
  scene_->setSelectionColor(QColor(dark ? "#60a5fa" : "#2563eb"));
  scene_->setEnergizedColor(settings_.energizedColor());
  scene_->setWireColor(settings_.wireColor());
  scene_->setGridVisible(settings_.gridVisible());
  scene_->setGridSize(settings_.gridSize());
  view_->viewport()->update();
}

QString MainWindow::documentDisplayName() const {
  if (document_ == nullptr) {
    return tr("Untitled");
  }
  if (!document_->filePath().isEmpty()) {
    return QFileInfo(document_->filePath()).completeBaseName();
  }
  if (!document_->diagram().metadata.title.empty()) {
    return QString::fromStdString(document_->diagram().metadata.title);
  }
  return tr("Untitled");
}

void MainWindow::updateWindowTitle() {
  if (document_ == nullptr) {
    return;
  }
  setWindowModified(document_->isModified());
  setWindowTitle(tr("%1[*] — FioVivo").arg(documentDisplayName()));
}

void MainWindow::updateSheetCombo() {
  if (sheet_combo_ == nullptr || document_ == nullptr) {
    return;
  }
  const QSignalBlocker blocker(sheet_combo_);
  sheet_combo_->clear();
  for (const Sheet& sheet : document_->diagram().sheets) {
    sheet_combo_->addItem(QString::fromStdString(sheet.name), sheet.id.value());
  }
  const SheetId current = document_->currentSheetId();
  for (int index = 0; index < sheet_combo_->count(); ++index) {
    if (sheet_combo_->itemData(index).toULongLong() == current.value()) {
      sheet_combo_->setCurrentIndex(index);
      break;
    }
  }
  sheet_label_->setText(scene_->sheetName());
}

void MainWindow::updateUndoActions() {
  if (save_action_ != nullptr) {
    save_action_->setEnabled(document_ != nullptr);
  }
}

void MainWindow::updateRecentMenu() {
  if (recent_menu_ == nullptr) {
    return;
  }
  recent_menu_->clear();
  const QStringList files = settings_.recentFiles();
  if (files.isEmpty()) {
    auto* empty = recent_menu_->addAction(tr("No recent files"));
    empty->setEnabled(false);
    return;
  }
  for (const QString& path : files) {
    auto* action = recent_menu_->addAction(QFileInfo(path).fileName());
    action->setToolTip(path);
    connect(action, &QAction::triggered, this, [this, path] {
      if (maybeSave()) {
        openPath(path);
      }
    });
  }
}

void MainWindow::updateStatusSimulation() {
  if (simulation_label_ == nullptr || controller_ == nullptr) {
    return;
  }
  QString state;
  switch (controller_->state()) {
  case SimulationController::State::Running:
    state = tr("Running");
    break;
  case SimulationController::State::Paused:
    state = tr("Paused");
    break;
  case SimulationController::State::Stopped:
    state = tr("Simulation stopped");
    break;
  }
  if (controller_->state() != SimulationController::State::Stopped) {
    state += QStringLiteral(" — ") + formatSimulatedTime(controller_->simulatedTimeMs());
  }
  simulation_label_->setText(state);
}

void MainWindow::updateSimulationUi() {
  if (controller_ == nullptr) {
    return;
  }
  switch (controller_->state()) {
  case SimulationController::State::Running:
    run_action_->setText(tr("&Pause Simulation"));
    run_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    run_action_->setToolTip(tr("Pause simulation (F5)"));
    break;
  case SimulationController::State::Paused:
    run_action_->setText(tr("&Resume Simulation"));
    run_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    run_action_->setToolTip(tr("Resume simulation (F5)"));
    break;
  case SimulationController::State::Stopped:
    run_action_->setText(tr("&Start Simulation"));
    run_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    run_action_->setToolTip(tr("Start simulation (F5)"));
    break;
  }
  const bool active = controller_->state() != SimulationController::State::Stopped;
  scene_->setSimulationInteractive(active);
  updateStatusSimulation();
}

void MainWindow::newProject() {
  if (!maybeSave()) {
    return;
  }
  NewProjectDialog dialog(examples_directory_, this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  const NewProjectOptions options = dialog.options();
  discardSimulation();
  const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  Diagram diagram;
  if (!options.template_path.isEmpty()) {
    const auto loaded = io::loadFile(options.template_path.toStdString());
    if (!loaded) {
      QMessageBox::critical(
          this, tr("Could not open template"), QString::fromStdString(loaded.error().toString()));
      return;
    }
    diagram = loaded.value();
    diagram.metadata.title = options.title.isEmpty() ? tr("Untitled").toStdString()
                                                     : options.title.toStdString();
    diagram.metadata.author = options.author.toStdString();
    diagram.metadata.created_utc = now.toStdString();
    diagram.metadata.modified_utc = now.toStdString();
    diagram.metadata.revision = "A";
    if (!diagram.sheets.empty()) {
      diagram.sheets.front().setup.paper = options.paper;
      diagram.sheets.front().setup.orientation = options.orientation;
    }
  } else {
    diagram.metadata.title = options.title.isEmpty() ? tr("Untitled").toStdString()
                                                     : options.title.toStdString();
    diagram.metadata.author = options.author.toStdString();
    diagram.metadata.created_utc = now.toStdString();
    diagram.metadata.modified_utc = now.toStdString();
    Sheet sheet;
    sheet.id = diagram.ids().newSheetId();
    sheet.name = tr("Sheet 1").toStdString();
    sheet.setup.paper = options.paper;
    sheet.setup.orientation = options.orientation;
    (void)diagram.addSheet(std::move(sheet));
  }
  document_->setDiagram(std::move(diagram), true);
  document_->setFilePath(QString(), true);
  updateWindowTitle();
  runValidation();
  updateSheetCombo();
  updateRecentMenu();
  QTimer::singleShot(0, view_, &SheetView::fitToPage);
  appendOutput(tr("New project created."));
}

void MainWindow::openFileDialog() {
  if (!maybeSave()) {
    return;
  }
  const QString directory =
      settings_.lastDirectory().isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
          : settings_.lastDirectory();
  const QString path = QFileDialog::getOpenFileName(
      this, tr("Open Project"), directory, tr("FioVivo projects (*.dcj);;All files (*)"));
  if (!path.isEmpty()) {
    openPath(path);
  }
}

bool MainWindow::openDocument(const QString& path) { return openPath(path); }

bool MainWindow::openPath(const QString& path) {
  const auto loaded = io::loadFile(path.toStdString());
  if (!loaded) {
    QMessageBox::critical(
        this,
        tr("Could not open project"),
        tr("%1\n\n%2").arg(path, QString::fromStdString(loaded.error().toString())));
    return false;
  }
  discardSimulation();
  document_->setDiagram(loaded.value(), true);
  document_->setFilePath(path, true);
  settings_.setLastDirectory(QFileInfo(path).absolutePath());
  settings_.addRecentFile(path);
  updateRecentMenu();
  updateSheetCombo();
  runValidation();
  updateWindowTitle();
  QTimer::singleShot(0, view_, &SheetView::fitToPage);
  appendOutput(tr("Opened %1").arg(path));
  return true;
}

bool MainWindow::saveFile() {
  if (document_->filePath().isEmpty()) {
    return saveFileAs();
  }
  return saveToPath(document_->filePath());
}

bool MainWindow::saveFileAs() {
  const QString directory =
      settings_.lastDirectory().isEmpty()
          ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
          : settings_.lastDirectory();
  QString path =
      QFileDialog::getSaveFileName(this,
                                   tr("Save Project As"),
                                   QDir(directory).filePath(documentDisplayName() + ".dcj"),
                                   tr("FioVivo projects (*.dcj);;All files (*)"));
  if (path.isEmpty()) {
    return false;
  }
  path = ensureSuffix(path, QStringLiteral(".dcj"));
  return saveToPath(path);
}

bool MainWindow::saveToPath(const QString& path) {
  document_->diagram().metadata.modified_utc =
      QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
  const auto saved = io::saveFile(document_->diagram(), path.toStdString());
  if (!saved) {
    QMessageBox::critical(
        this, tr("Could not save project"), QString::fromStdString(saved.error().toString()));
    return false;
  }
  document_->setFilePath(path, true);
  settings_.setLastDirectory(QFileInfo(path).absolutePath());
  settings_.addRecentFile(path);
  updateRecentMenu();
  updateWindowTitle();
  clearRecovery();
  appendOutput(tr("Saved %1").arg(path));
  return true;
}

bool MainWindow::maybeSave() {
  if (!document_->isModified()) {
    return true;
  }
  const QMessageBox::StandardButton answer =
      QMessageBox::warning(this,
                           tr("Unsaved changes"),
                           tr("The project \"%1\" has unsaved changes. Do you want to save them?")
                               .arg(documentDisplayName()),
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                           QMessageBox::Save);
  if (answer == QMessageBox::Cancel) {
    return false;
  }
  if (answer == QMessageBox::Save) {
    return saveFile();
  }
  return true;
}

void MainWindow::insertTextNote() {
  const QPointF position = scene_->snap(view_->mapToScene(view_->viewport()->rect().center()));
  Annotation annotation = makeTextAnnotation(
      document_->diagram(), Vec2{position.x(), position.y()}, tr("Text note").toStdString());
  document_->mutate(tr("Add text note"), [&](Diagram& diagram) {
    (void)diagram.addAnnotation(document_->currentSheetId(), annotation);
  });
}

void MainWindow::insertRectangle() {
  const QPointF position = scene_->snap(view_->mapToScene(view_->viewport()->rect().center()));
  Annotation annotation = makeRectangleAnnotation(
      document_->diagram(), Vec2{position.x(), position.y()}, Vec2{60.0, 30.0});
  document_->mutate(tr("Add rectangle"), [&](Diagram& diagram) {
    (void)diagram.addAnnotation(document_->currentSheetId(), annotation);
  });
}

void MainWindow::addSheet() {
  const Sheet* current = document_->currentSheet();
  if (current == nullptr) {
    return;
  }
  Sheet sheet;
  sheet.id = document_->diagram().ids().newSheetId();
  sheet.name = tr("Sheet %1").arg(document_->diagram().sheets.size() + 1).toStdString();
  sheet.setup = current->setup;
  const SheetId new_id = sheet.id;
  document_->mutate(tr("Add sheet"),
                    [&](Diagram& diagram) { (void)diagram.addSheet(std::move(sheet)); });
  document_->setCurrentSheet(new_id);
  updateSheetCombo();
  QTimer::singleShot(0, view_, &SheetView::fitToPage);
}

void MainWindow::deleteSelection() { scene_->deleteSelection(); }
void MainWindow::copySelection() { scene_->copySelection(); }
void MainWindow::cutSelection() {
  scene_->copySelection();
  scene_->deleteSelection();
}
void MainWindow::pasteSelection() { scene_->pasteClipboard(); }
void MainWindow::duplicateSelection() { scene_->duplicateSelection(); }
void MainWindow::selectAll() { scene_->selectAllContent(); }

void MainWindow::startOrPauseSimulation() {
  if (controller_->state() == SimulationController::State::Running) {
    controller_->pause();
    return;
  }
  if (controller_->state() == SimulationController::State::Paused) {
    controller_->start();
    return;
  }
  if (!ensureSimulationLoaded()) {
    return;
  }
  log_dock_->clear();
  controller_->start();
  updateSimulationUi();
}

bool MainWindow::ensureSimulationLoaded() {
  if (controller_->state() != SimulationController::State::Stopped && controller_->isLoaded()) {
    return true;
  }
  const ValidationReport report = validate(document_->diagram());
  problems_dock_->setReport(report);
  if (report.hasErrors()) {
    bottom_tabs_->setCurrentWidget(problems_dock_);
    bottom_panel_->show();
    QMessageBox::warning(
        this,
        tr("Simulation blocked"),
        tr("Fix the errors in the Problems panel before starting the simulation."));
    return false;
  }
  const double speed = speed_combo_ != nullptr ? speed_combo_->currentData().toDouble()
                                               : settings_.simulationSpeed();
  controller_->loadDiagram(document_->diagram(), settings_.cycleMs(), speed);
  scene_->setSimulationInteractive(true);
  updateSimulationUi();
  return true;
}

void MainWindow::resetSimulation() {
  if (controller_ == nullptr) {
    return;
  }
  controller_->resetSimulation();
  scene_->clearSimulation();
  scene_->setSimulationInteractive(false);
  log_dock_->clear();
  updateSimulationUi();
  appendOutput(tr("Simulation reset to the initial state."));
}

void MainWindow::stepSimulation() {
  if (controller_->state() == SimulationController::State::Running) {
    controller_->pause();
  }
  if (!ensureSimulationLoaded()) {
    return;
  }
  controller_->stepOnce();
  updateSimulationUi();
}

void MainWindow::stepCycles() {
  bool ok = false;
  const int count = QInputDialog::getInt(
      this, tr("Step Cycles"), tr("Number of scan cycles:"), 10, 1, 100000, 1, &ok);
  if (!ok) {
    return;
  }
  if (!ensureSimulationLoaded()) {
    return;
  }
  controller_->stepCycles(count);
  updateSimulationUi();
}

void MainWindow::resetEmergencyStops() {
  if (controller_ == nullptr || !controller_->isLoaded()) {
    showStatusMessage(tr("Start the simulation first."));
    return;
  }
  for (const Sheet& sheet : document_->diagram().sheets) {
    for (const Component& component : sheet.components) {
      if (component.type == component_type::kEmergencyStop) {
        controller_->setInput(component.id.value(), false);
      }
    }
  }
  appendOutput(tr("Emergency stop(s) reset."));
}

void MainWindow::discardSimulation() {
  if (controller_ == nullptr) {
    return;
  }
  controller_->discard();
  scene_->clearSimulation();
  scene_->setSimulationInteractive(false);
  updateSimulationUi();
}

void MainWindow::handleInputRequest(quint64 component, bool active) {
  if (controller_->state() == SimulationController::State::Stopped) {
    showStatusMessage(tr("Start or pause the simulation to operate this input."));
    return;
  }
  controller_->setInput(component, active);
}

void MainWindow::handleTripRequest(quint64 component, bool tripped) {
  if (controller_->state() == SimulationController::State::Stopped) {
    showStatusMessage(tr("Start or pause the simulation to trip this device."));
    return;
  }
  controller_->setTripped(component, tripped);
}

void MainWindow::handleSelectorRequest(quint64 component, int position) {
  if (controller_->state() == SimulationController::State::Stopped) {
    showStatusMessage(tr("Start or pause the simulation to operate the selector."));
    return;
  }
  controller_->setSelector(component, position);
}

void MainWindow::handleSimulationFrame(const SimFrame& frame) {
  scene_->setSimulationFrame(frame);
  properties_dock_->updateSimulation(frame);
  updateStatusSimulation();
}

void MainWindow::handleSimulationState() {
  updateSimulationUi();
  if (controller_->state() == SimulationController::State::Stopped) {
    scene_->setSimulationInteractive(false);
  }
}

void MainWindow::handleResetOccurred() {
  log_dock_->clear();
  if (controller_->state() == SimulationController::State::Stopped) {
    scene_->clearSimulation();
    scene_->setSimulationInteractive(false);
    appendOutput(tr("Simulation reset."));
  }
  updateStatusSimulation();
}

void MainWindow::runValidation() {
  if (document_ == nullptr) {
    return;
  }
  const ValidationReport report = validate(document_->diagram());
  problems_dock_->setReport(report);
  statusBar()->showMessage(tr("Validation: %1 error(s), %2 warning(s)")
                               .arg(static_cast<qulonglong>(report.errorCount()))
                               .arg(static_cast<qulonglong>(report.warningCount())),
                           5000);
}

void MainWindow::navigateTo(quint64 component, bool has_component, quint64 wire) {
  if (has_component) {
    const ComponentId id{component};
    if (const Sheet* sheet = document_->diagram().findSheetForComponent(id)) {
      if (sheet->id != document_->currentSheetId()) {
        document_->setCurrentSheet(sheet->id);
      }
    }
    scene_->selectComponent(id);
    view_->centerOnComponent(id);
    properties_panel_->show();
    properties_panel_->raise();
    return;
  }
  const WireId wire_id{wire};
  if (const Wire* item = document_->diagram().findWire(wire_id)) {
    if (const Sheet* sheet = document_->diagram().findSheetForComponent(item->from.component)) {
      if (sheet->id != document_->currentSheetId()) {
        document_->setCurrentSheet(sheet->id);
      }
    }
  }
  scene_->selectWire(wire_id);
  view_->centerOnWire(wire_id);
  properties_panel_->show();
  properties_panel_->raise();
}

void MainWindow::showPreferences() {
  PreferencesDialog dialog(settings_, this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  applyTheme();
  if (speed_combo_ != nullptr) {
    const int index = speed_combo_->findData(settings_.simulationSpeed());
    if (index >= 0) {
      QSignalBlocker blocker(speed_combo_);
      speed_combo_->setCurrentIndex(index);
    }
  }
  if (controller_->isLoaded()) {
    discardSimulation();
  }
  appendOutput(tr("Preferences updated."));
}

void MainWindow::exportPng() {
  PngOptionsDialog options_dialog(this);
  if (options_dialog.exec() != QDialog::Accepted) {
    return;
  }
  QString path = QFileDialog::getSaveFileName(
      this,
      tr("Export PNG"),
      QDir(defaultExportDirectory()).filePath(documentDisplayName() + ".png"),
      tr("PNG image (*.png);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }
  path = ensureSuffix(path, QStringLiteral(".png"));
  QString error;
  if (fiovivo::ui::exportPng(*scene_, path, options_dialog.options(), &error)) {
    appendOutput(tr("PNG exported to %1").arg(path));
    showStatusMessage(tr("PNG exported."));
  } else {
    QMessageBox::critical(this, tr("Export failed"), error);
  }
}

void MainWindow::exportSvg() {
  QString path = QFileDialog::getSaveFileName(
      this,
      tr("Export SVG"),
      QDir(defaultExportDirectory()).filePath(documentDisplayName() + ".svg"),
      tr("SVG image (*.svg);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }
  path = ensureSuffix(path, QStringLiteral(".svg"));
  QString error;
  if (fiovivo::ui::exportSvg(*scene_, path, false, &error)) {
    appendOutput(tr("SVG exported to %1").arg(path));
    showStatusMessage(tr("SVG exported."));
  } else {
    QMessageBox::critical(this, tr("Export failed"), error);
  }
}

void MainWindow::exportPdf() {
  QString path = QFileDialog::getSaveFileName(
      this,
      tr("Export PDF"),
      QDir(defaultExportDirectory()).filePath(documentDisplayName() + ".pdf"),
      tr("PDF document (*.pdf);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }
  path = ensureSuffix(path, QStringLiteral(".pdf"));
  QString error;
  if (fiovivo::ui::exportPdf(*scene_, path, &error)) {
    appendOutput(tr("PDF exported to %1").arg(path));
    showStatusMessage(tr("PDF exported."));
  } else {
    QMessageBox::critical(this, tr("Export failed"), error);
  }
}

void MainWindow::exportBom() {
  QString path = QFileDialog::getSaveFileName(
      this,
      tr("Export Bill of Materials"),
      QDir(defaultExportDirectory()).filePath(documentDisplayName() + "-bom.csv"),
      tr("CSV files (*.csv);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }
  path = ensureSuffix(path, QStringLiteral(".csv"));
  const auto csv = io::bomToCsv(document_->diagram());
  if (!csv) {
    QMessageBox::critical(
        this, tr("Export failed"), QString::fromStdString(csv.error().toString()));
    return;
  }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, tr("Export failed"), tr("Could not write %1").arg(path));
    return;
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  stream << QString::fromStdString(csv.value());
  appendOutput(tr("Bill of materials exported to %1").arg(path));
  showStatusMessage(tr("BOM exported."));
}

void MainWindow::exportSimulationLog() {
  const QString path = QFileDialog::getSaveFileName(
      this,
      tr("Export Simulation Log"),
      QDir(defaultExportDirectory()).filePath(QStringLiteral("simulation-log.csv")),
      tr("CSV files (*.csv);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }
  if (log_dock_->exportCsv(ensureSuffix(path, QStringLiteral(".csv")))) {
    showStatusMessage(tr("Simulation log exported."));
  } else {
    QMessageBox::warning(this, tr("Export failed"), tr("Could not export the simulation log."));
  }
}

void MainWindow::printPreview() {
  QPrinter printer(QPrinter::HighResolution);
  QPrintPreviewDialog preview(&printer, this);
  preview.setWindowTitle(tr("Print Preview"));
  connect(&preview, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* target) {
    QString error;
    if (!printScene(*scene_, *target, &error)) {
      appendOutput(error);
    }
  });
  preview.exec();
}

void MainWindow::printDocument() {
  QPrinter printer(QPrinter::HighResolution);
  QPrintDialog dialog(&printer, this);
  dialog.setWindowTitle(tr("Print Diagram"));
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }
  QString error;
  if (!printScene(*scene_, printer, &error)) {
    QMessageBox::critical(this, tr("Print failed"), error);
  }
}

QString MainWindow::recoveryDirectory() const {
  const QString path =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/recovery";
  QDir().mkpath(path);
  return path;
}

QString MainWindow::defaultExportDirectory() const {
  if (!settings_.lastDirectory().isEmpty()) {
    return settings_.lastDirectory();
  }
  return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void MainWindow::setupAutosave() {
  autosave_timer_ = new QTimer(this);
  autosave_timer_->setInterval(kAutosaveIntervalMs);
  connect(autosave_timer_, &QTimer::timeout, this, &MainWindow::autosave);
  autosave_timer_->start();
}

void MainWindow::autosave() {
  if (document_ == nullptr || !document_->isModified()) {
    return;
  }
  const QString directory = recoveryDirectory();
  const QString data_path = QDir(directory).filePath(QStringLiteral("recovery.dcj"));
  const QString info_path = QDir(directory).filePath(QStringLiteral("recovery.json"));
  const auto saved = io::saveFile(document_->diagram(), data_path.toStdString());
  if (!saved) {
    return;
  }
  QJsonObject info;
  info["original_path"] = document_->filePath();
  info["title"] = QString::fromStdString(document_->diagram().metadata.title);
  info["saved_utc"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
  QFile file(info_path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    file.write(QJsonDocument(info).toJson(QJsonDocument::Indented));
  }
  appendOutput(tr("Recovery copy written."));
}

void MainWindow::checkRecovery() {
  const QString directory = recoveryDirectory();
  const QString data_path = QDir(directory).filePath(QStringLiteral("recovery.dcj"));
  const QString info_path = QDir(directory).filePath(QStringLiteral("recovery.json"));
  if (!QFileInfo::exists(data_path) || !QFileInfo::exists(info_path)) {
    return;
  }
  QFile info_file(info_path);
  if (!info_file.open(QIODevice::ReadOnly)) {
    return;
  }
  const QJsonDocument info_document = QJsonDocument::fromJson(info_file.readAll());
  if (!info_document.isObject()) {
    return;
  }
  const QJsonObject info = info_document.object();
  const QString original = info.value("original_path").toString();
  const QFileInfo recovery_info(data_path);
  if (!original.isEmpty() && QFileInfo::exists(original)) {
    const QFileInfo original_info(original);
    if (recovery_info.lastModified() <= original_info.lastModified()) {
      return;
    }
  }
  const QMessageBox::StandardButton answer =
      QMessageBox::question(this,
                            tr("Recover unsaved work?"),
                            tr("An autosave from %1 was found. Do you want to restore it?")
                                .arg(info.value("saved_utc").toString()),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);
  if (answer != QMessageBox::Yes) {
    return;
  }
  const auto recovered = io::loadFile(data_path.toStdString());
  if (!recovered) {
    QMessageBox::warning(
        this, tr("Recovery failed"), QString::fromStdString(recovered.error().toString()));
    return;
  }
  document_->setDiagram(recovered.value(), true);
  if (!original.isEmpty()) {
    document_->setFilePath(original, true);
  }
  document_->setModified(true);
  document_->setCurrentSheet(document_->diagram().sheets.front().id);
  updateWindowTitle();
  updateSheetCombo();
  runValidation();
  QTimer::singleShot(0, view_, &SheetView::fitToPage);
  appendOutput(tr("Recovered unsaved work."));
}

void MainWindow::clearRecovery() {
  const QString directory = recoveryDirectory();
  QFile::remove(QDir(directory).filePath(QStringLiteral("recovery.dcj")));
  QFile::remove(QDir(directory).filePath(QStringLiteral("recovery.json")));
}

void MainWindow::appendOutput(const QString& message) {
  if (output_ == nullptr) {
    return;
  }
  const QString timestamp = QTime::currentTime().toString(QStringLiteral("HH:mm:ss"));
  output_->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
}

void MainWindow::showStatusMessage(const QString& message) {
  statusBar()->showMessage(message, 6000);
}

void MainWindow::showAbout() {
  AboutDialog dialog(this);
  dialog.exec();
}

void MainWindow::showDocumentation() {
  QDesktopServices::openUrl(
      QUrl(QStringLiteral("https://github.com/RodrigoKuerten/electrical-diagram/tree/main/docs")));
}

void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  if (!shown_once_) {
    shown_once_ = true;
    QTimer::singleShot(300, this, [this] { view_->fitToPage(); });
  }
}

void MainWindow::closeEvent(QCloseEvent* event) {
  if (!maybeSave()) {
    event->ignore();
    return;
  }
  settings_.setWindowGeometry(saveGeometry());
  settings_.setWindowState(saveState());
  if (document_->isModified()) {
    autosave();
  } else {
    clearRecovery();
  }
  if (autosave_timer_ != nullptr) {
    autosave_timer_->stop();
  }
  controller_->discard();
  event->accept();
}

} // namespace fiovivo::ui
