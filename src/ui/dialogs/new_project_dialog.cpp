#include "ui/dialogs/new_project_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include <algorithm>

namespace fiovivo::ui {

NewProjectDialog::NewProjectDialog(const QString& examples_directory, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle(tr("New Project"));
  setMinimumWidth(440);
  auto* layout = new QVBoxLayout(this);
  auto* form = new QFormLayout();
  title_ = new QLineEdit(tr("Untitled"), this);
  author_ = new QLineEdit(this);
  paper_ = new QComboBox(this);
  for (PaperSize size :
       {PaperSize::A4, PaperSize::A3, PaperSize::A2, PaperSize::A1, PaperSize::Letter}) {
    paper_->addItem(QString::fromStdString(std::string(toString(size))), static_cast<int>(size));
  }
  orientation_ = new QComboBox(this);
  orientation_->addItem(tr("Landscape"), static_cast<int>(Orientation::Landscape));
  orientation_->addItem(tr("Portrait"), static_cast<int>(Orientation::Portrait));
  template_ = new QComboBox(this);
  template_->addItem(tr("Blank"), QString());
  if (!examples_directory.isEmpty()) {
    QDir directory(examples_directory);
    const QFileInfoList entries = directory.entryInfoList({"*.dcj"}, QDir::Files, QDir::Name);
    for (const QFileInfo& entry : entries) {
      template_->addItem(entry.completeBaseName(), entry.absoluteFilePath());
    }
  }
  form->addRow(tr("Title"), title_);
  form->addRow(tr("Author"), author_);
  form->addRow(tr("Paper size"), paper_);
  form->addRow(tr("Orientation"), orientation_);
  form->addRow(tr("Template"), template_);
  layout->addLayout(form);

  auto* hint = new QLabel(tr("The template creates a new project with the selected sheet setup. "
                             "You can change it later in the sheet settings."),
                          this);
  hint->setWordWrap(true);
  layout->addWidget(hint);

  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

NewProjectOptions NewProjectDialog::options() const {
  NewProjectOptions result;
  result.title = title_->text().trimmed();
  result.author = author_->text().trimmed();
  result.paper = static_cast<PaperSize>(paper_->currentData().toInt());
  result.orientation = static_cast<Orientation>(orientation_->currentData().toInt());
  result.template_path = template_->currentData().toString();
  return result;
}

} // namespace fiovivo::ui
