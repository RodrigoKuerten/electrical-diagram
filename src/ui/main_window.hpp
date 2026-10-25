#pragma once

#include "ui/sim_ui_types.hpp"

#include <QMainWindow>
#include <QString>

#include <memory>

class QAction;
class QComboBox;
class QDockWidget;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QTabWidget;
class QTimer;

namespace fiovivo {
class SymbolLibrary;
class SymbolRenderCache;
} // namespace fiovivo

namespace fiovivo::ui {

class AppSettings;
class Document;
class LibraryDock;
class ProblemsDock;
class PropertiesDock;
class SheetScene;
class SheetView;
class SimulationController;
class SimulationLogDock;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QString asset_root, AppSettings& settings, QWidget* parent = nullptr);
  ~MainWindow() override;

  [[nodiscard]] bool isValid() const { return initialized_; }
  bool openDocument(const QString& path);

protected:
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void initializeSymbols();
  void createCentralView();
  void createActions();
  void createMenus();
  void createToolbar();
  void createDocks();
  void createStatusBar();
  void connectSignals();
  void applyTheme();
  void updateThemeIcons();
  void updateWindowTitle();
  void updateSheetCombo();
  void updateUndoActions();
  void updateSimulationUi();
  void updateRecentMenu();
  void updateStatusSimulation();

  void newProject();
  void openFileDialog();
  bool openPath(const QString& path);
  bool saveFile();
  bool saveFileAs();
  bool saveToPath(const QString& path);
  bool maybeSave();
  void closeProject();

  void insertTextNote();
  void insertRectangle();
  void addSheet();
  void deleteSelection();
  void copySelection();
  void cutSelection();
  void pasteSelection();
  void duplicateSelection();
  void selectAll();

  void startOrPauseSimulation();
  void resetSimulation();
  void stepSimulation();
  void stepCycles();
  void resetEmergencyStops();
  void discardSimulation();
  [[nodiscard]] bool ensureSimulationLoaded();
  void handleInputRequest(quint64 component, bool active);
  void handleTripRequest(quint64 component, bool tripped);
  void handleSelectorRequest(quint64 component, int position);
  void handleSimulationFrame(const SimFrame& frame);
  void handleSimulationState();
  void handleResetOccurred();

  void runValidation();
  void navigateTo(quint64 component, bool has_component, quint64 wire);
  void showPreferences();
  void showWorkspaceSizeDialog();
  void resizeWorkspaceBy(double dw, double dh);

  void exportPng();
  void exportSvg();
  void exportPdf();
  void exportBom();
  void exportSimulationLog();
  void printPreview();
  void printDocument();

  void setupAutosave();
  void autosave();
  void checkRecovery();
  void clearRecovery();
  void restoreRecovery();

  void appendOutput(const QString& message);
  void showStatusMessage(const QString& message);
  void showAbout();
  void showDocumentation();

  [[nodiscard]] QString recoveryDirectory() const;
  [[nodiscard]] QString defaultExportDirectory() const;
  [[nodiscard]] QString documentDisplayName() const;

  QString asset_root_;
  QString examples_directory_;
  AppSettings& settings_;
  std::unique_ptr<SymbolLibrary> library_;
  std::unique_ptr<SymbolRenderCache> render_cache_;
  bool initialized_ = false;

  Document* document_ = nullptr;
  SheetScene* scene_ = nullptr;
  SheetView* view_ = nullptr;
  SimulationController* controller_ = nullptr;

  LibraryDock* library_dock_ = nullptr;
  PropertiesDock* properties_dock_ = nullptr;
  ProblemsDock* problems_dock_ = nullptr;
  SimulationLogDock* log_dock_ = nullptr;
  QPlainTextEdit* output_ = nullptr;
  QTabWidget* bottom_tabs_ = nullptr;
  QDockWidget* library_panel_ = nullptr;
  QDockWidget* properties_panel_ = nullptr;
  QDockWidget* bottom_panel_ = nullptr;

  QLabel* zoom_label_ = nullptr;
  QLabel* sheet_label_ = nullptr;
  QLabel* cursor_label_ = nullptr;
  QLabel* simulation_label_ = nullptr;
  QComboBox* sheet_combo_ = nullptr;
  QComboBox* speed_combo_ = nullptr;
  QTimer* autosave_timer_ = nullptr;

  QAction* new_action_ = nullptr;
  QAction* open_action_ = nullptr;
  QAction* save_action_ = nullptr;
  QAction* save_as_action_ = nullptr;
  QAction* exit_action_ = nullptr;
  QAction* undo_action_ = nullptr;
  QAction* redo_action_ = nullptr;
  QAction* cut_action_ = nullptr;
  QAction* copy_action_ = nullptr;
  QAction* paste_action_ = nullptr;
  QAction* duplicate_action_ = nullptr;
  QAction* delete_action_ = nullptr;
  QAction* select_all_action_ = nullptr;
  QAction* zoom_in_action_ = nullptr;
  QAction* zoom_out_action_ = nullptr;
  QAction* fit_action_ = nullptr;
  QAction* fit_workspace_action_ = nullptr;
  QAction* workspace_size_action_ = nullptr;
  QAction* workspace_wider_action_ = nullptr;
  QAction* workspace_narrower_action_ = nullptr;
  QAction* workspace_taller_action_ = nullptr;
  QAction* workspace_shorter_action_ = nullptr;
  QAction* grid_action_ = nullptr;
  QAction* wire_mode_action_ = nullptr;
  QAction* run_action_ = nullptr;
  QAction* step_action_ = nullptr;
  QAction* step_n_action_ = nullptr;
  QAction* reset_action_ = nullptr;
  QAction* reset_estop_action_ = nullptr;
  QAction* validate_action_ = nullptr;
  QAction* preferences_action_ = nullptr;
  QAction* about_action_ = nullptr;
  QMenu* recent_menu_ = nullptr;

  bool closing_ = false;
  bool shown_once_ = false;
};

} // namespace fiovivo::ui
