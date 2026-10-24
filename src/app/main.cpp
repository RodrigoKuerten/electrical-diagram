#include "ui/app_settings.hpp"
#include "ui/asset_locator.hpp"
#include "ui/main_window.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

int main(int argc, char** argv) {
#ifdef Q_OS_WIN
  // The native Windows menu bar follows the operating-system theme, which would contradict an
  // in-application light/dark choice. Draw the menu bar with the active theme palette instead.
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif
  QApplication application(argc, argv);
  QApplication::setOrganizationName(QStringLiteral("FioVivo"));
  QApplication::setOrganizationDomain(QStringLiteral("fiovivo.local"));
  QApplication::setApplicationName(QStringLiteral("FioVivo"));
  QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
  QApplication::setApplicationDisplayName(QStringLiteral("FioVivo"));

  fiovivo::ui::AppSettings settings;
  const QString language = settings.language();
  QLocale locale;
  if (language == "pt_BR") {
    locale = QLocale(QLocale::Portuguese, QLocale::Brazil);
  } else {
    locale = QLocale(QLocale::English, QLocale::UnitedStates);
  }
  QLocale::setDefault(locale);

  QTranslator translator;
  const QString translation = fiovivo::ui::findTranslationFile(language);
  if (!translation.isEmpty()) {
    (void)translator.load(translation);
    application.installTranslator(&translator);
  }
  settings.applyAppearance(application);

  const QString assets = fiovivo::ui::findAssetRoot();
  if (assets.isEmpty()) {
    QMessageBox::critical(nullptr,
                          QObject::tr("FioVivo"),
                          QObject::tr("The symbol assets were not found. Reinstall the "
                                      "application or set FIOVIVO_ASSET_DIR."));
    return 1;
  }

  fiovivo::ui::MainWindow window(assets, settings);
  if (!window.isValid()) {
    return 1;
  }
  const QStringList arguments = application.arguments();
  if (arguments.size() > 1 && QFileInfo::exists(arguments.at(1))) {
    window.openDocument(arguments.at(1));
  }
  window.show();
  return application.exec();
}
