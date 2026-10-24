// Captures the theme-sensitive surfaces of the editor for the Track A audit.
// Usage: fiovivo_theme_screenshots --theme light|dark --out <directory>

#include "core/model.hpp"
#include "ui/app_settings.hpp"
#include "ui/dialogs/about_dialog.hpp"
#include "ui/dialogs/preferences_dialog.hpp"
#include "ui/document.hpp"
#include "ui/library_dock.hpp"
#include "ui/main_window.hpp"
#include "ui/problems_dock.hpp"
#include "ui/properties_dock.hpp"
#include "ui/sheet_scene.hpp"
#include "ui/sheet_view.hpp"
#include "ui/simulation_log_dock.hpp"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QThread>
#include <QToolBar>
#include <QWidget>

#include <cstdio>
#include <string>

namespace {

void waitForUi(int milliseconds) {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < milliseconds) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    QThread::msleep(10);
  }
  QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
}

[[nodiscard]] QString argumentValue(const QStringList& arguments, const QString& name) {
  const qsizetype index = arguments.indexOf(name);
  if (index < 0 || index + 1 >= arguments.size()) {
    return {};
  }
  return arguments.at(index + 1);
}

[[nodiscard]] bool saveWidget(QWidget* widget, const QString& directory, const QString& name) {
  if (widget == nullptr) {
    std::fprintf(stderr, "missing widget for %s\n", name.toUtf8().constData());
    return false;
  }
  const QPixmap pixmap = widget->grab();
  if (pixmap.isNull()) {
    std::fprintf(stderr, "empty pixmap for %s\n", name.toUtf8().constData());
    return false;
  }
  const QString path = QDir(directory).filePath(name + ".png");
  if (!pixmap.save(path, "PNG")) {
    std::fprintf(stderr, "could not save %s\n", path.toUtf8().constData());
    return false;
  }
  std::printf("wrote %s (%dx%d)\n", path.toUtf8().constData(), pixmap.width(), pixmap.height());
  return true;
}

[[nodiscard]] QMenu* findMenu(QMenuBar* menu_bar, const QString& keyword) {
  if (menu_bar == nullptr) {
    return nullptr;
  }
  for (QAction* action : menu_bar->actions()) {
    if (action->menu() != nullptr && action->text().contains(keyword, Qt::CaseInsensitive)) {
      return action->menu();
    }
  }
  return nullptr;
}

} // namespace

int main(int argc, char** argv) {
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
  QApplication application(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("FioVivoAudit"));
  QCoreApplication::setApplicationName(QStringLiteral("FioVivoAudit"));
  QApplication::setApplicationName(QStringLiteral("FioVivo"));
  QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

  const QStringList arguments = QCoreApplication::arguments();
  const QString theme = argumentValue(arguments, QStringLiteral("--theme")).isEmpty()
                            ? QStringLiteral("light")
                            : argumentValue(arguments, QStringLiteral("--theme"));
  const QString out_directory = argumentValue(arguments, QStringLiteral("--out"));
  if (out_directory.isEmpty()) {
    std::fprintf(stderr, "usage: fiovivo_theme_screenshots --theme light|dark --out <dir>\n");
    return 2;
  }
  if (!QDir().mkpath(out_directory)) {
    std::fprintf(stderr, "could not create %s\n", out_directory.toUtf8().constData());
    return 2;
  }

  fiovivo::ui::AppSettings settings;
  settings.setTheme(theme);
  settings.setFontPointSize(10);
  settings.setGridVisible(true);
  settings.applyAppearance(application);

  const QString asset_root = QString::fromUtf8(FIOVIVO_ASSET_DIR);
  fiovivo::ui::MainWindow window(asset_root, settings);
  if (!window.isValid()) {
    std::fprintf(
        stderr, "main window failed to initialize with %s\n", asset_root.toUtf8().constData());
    return 3;
  }
  window.resize(1380, 880);
  window.show();
  waitForUi(250);

  const QString example = QString::fromUtf8(FIOVIVO_EXAMPLES_DIR) + "/partida-direta.dcj";
  if (QFileInfo::exists(example)) {
    window.openDocument(example);
    waitForUi(250);
  }
  if (auto* scene = window.findChild<fiovivo::ui::SheetScene*>()) {
    if (const fiovivo::Sheet* sheet = scene->document()->currentSheet();
        sheet != nullptr && !sheet->components.empty()) {
      scene->selectComponent(sheet->components.front().id);
      waitForUi(100);
    }
  }
  waitForUi(100);

  bool ok = true;
  ok = saveWidget(&window, out_directory, QStringLiteral("01-full-window")) && ok;
  ok = saveWidget(window.findChild<fiovivo::ui::SheetView*>(),
                  out_directory,
                  QStringLiteral("02-canvas")) &&
       ok;
  ok = saveWidget(window.findChild<QToolBar*>(), out_directory, QStringLiteral("03-toolbar")) && ok;
  ok = saveWidget(window.statusBar(), out_directory, QStringLiteral("04-status-bar")) && ok;
  ok = saveWidget(window.findChild<fiovivo::ui::LibraryDock*>(),
                  out_directory,
                  QStringLiteral("05-library-dock")) &&
       ok;
  ok = saveWidget(window.findChild<fiovivo::ui::PropertiesDock*>(),
                  out_directory,
                  QStringLiteral("06-properties-dock")) &&
       ok;
  ok = saveWidget(window.findChild<fiovivo::ui::ProblemsDock*>(),
                  out_directory,
                  QStringLiteral("07-problems-dock")) &&
       ok;
  if (auto* tabs = window.findChild<QTabWidget*>(); tabs != nullptr) {
    if (auto* log = window.findChild<fiovivo::ui::SimulationLogDock*>(); log != nullptr) {
      tabs->setCurrentWidget(log);
      waitForUi(120);
      ok = saveWidget(&window, out_directory, QStringLiteral("08-simulation-log")) && ok;
      ok = saveWidget(log, out_directory, QStringLiteral("08b-simulation-log-dock")) && ok;
      if (auto* problems = window.findChild<fiovivo::ui::ProblemsDock*>(); problems != nullptr) {
        tabs->setCurrentWidget(problems);
      }
    }
  }

  if (QMenu* menu = findMenu(window.menuBar(), QStringLiteral("View"))) {
    menu->popup(window.mapToGlobal(QPoint(80, 70)));
    waitForUi(120);
    ok = saveWidget(menu, out_directory, QStringLiteral("09-view-menu")) && ok;
    menu->close();
  }

  fiovivo::ui::PreferencesDialog preferences(settings, &window);
  preferences.show();
  waitForUi(120);
  ok = saveWidget(&preferences, out_directory, QStringLiteral("10-preferences-dialog")) && ok;
  preferences.close();

  fiovivo::ui::AboutDialog about(&window);
  about.show();
  waitForUi(120);
  ok = saveWidget(&about, out_directory, QStringLiteral("11-about-dialog")) && ok;
  about.close();

  window.close();
  return ok ? 0 : 4;
}
