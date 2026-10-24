#include "ui/dialogs/about_dialog.hpp"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

namespace fiovivo::ui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(tr("About FioVivo"));
  setMinimumWidth(540);
  auto* layout = new QVBoxLayout(this);

  auto* title = new QLabel(
      QStringLiteral("<h2>FioVivo %1</h2>").arg(QApplication::applicationVersion()), this);
  layout->addWidget(title);

  auto* description = new QLabel(
      tr("Educational electrical control diagram editor and simulator.<br>"
         "Working title: FioVivo. Symbol artwork is original and follows common IEC 60617 "
         "and Brazilian NBR 5280-style conventions."),
      this);
  description->setWordWrap(true);
  layout->addWidget(description);

  auto* warning = new QLabel(
      tr("<b>Safety disclaimer:</b> this is an educational and prototyping tool. It is not a "
         "certified safety tool and must not be used to design, verify or commission "
         "safety-critical electrical systems. Voltage, current, heating, mechanical motion and "
         "electrical safety are not simulated."),
      this);
  warning->setWordWrap(true);
  warning->setStyleSheet(QStringLiteral("color:#b91c1c;"));
  layout->addWidget(warning);

  auto* license = new QLabel(
      tr("<b>License:</b> MIT License. Copyright (c) 2026 Rodrigo Kuerten.<br><br>"
         "<b>Third-party libraries:</b><br>"
         "Qt 6 — LGPL-3.0 (or commercial license) — GUI, SVG, printing and translations.<br>"
         "nlohmann/json — MIT — JSON persistence.<br>"
         "GoogleTest — BSD-3-Clause — tests only.<br><br>"
         "Source repository: <a href=\"https://github.com/RodrigoKuerten/electrical-diagram\">"
         "github.com/RodrigoKuerten/electrical-diagram</a>"),
      this);
  license->setWordWrap(true);
  license->setOpenExternalLinks(true);
  license->setTextFormat(Qt::RichText);
  layout->addWidget(license);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

} // namespace fiovivo::ui
