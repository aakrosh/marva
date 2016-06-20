#include "start_dialog.h"
#include "ui_start_dialog.h"
#include "history.h"

#include <QFileDialog>
#include <QMessageBox>

StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartDialog)
{
    ui->setupUi(this);
    ui->bLoadLast->setFocus();
    History *h = AbstractConfigFileFactory<History>::create(this);
    ui->bLoadLast->setEnabled(!h->lastProject().isEmpty());
}

StartDialog::~StartDialog()
{
    delete ui;
}

void StartDialog::openLastProject()
{
    History *h = AbstractConfigFileFactory<History>::create(this);;
    EmitFileNameAndAccept(h->lastProject());
}

void StartDialog::EmitFileNameAndAccept(QString projectFileName)
{
    setEnabled(false);
    setVisible(false);
    QMessageBox *msg = new QMessageBox(QMessageBox::NoIcon, "Please wait", "Taxonomy tree is loading", QMessageBox::NoButton, this,
                                   Qt::WindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint);
    msg->setStandardButtons(0);
    msg->show();
    QApplication::processEvents();
    emit fileChoosen(projectFileName);
    accept();
}

void StartDialog::openProjectFile()
{
    QString projectFileName = QFileDialog::getOpenFileName(this, tr("Open project"),
                                                          QString(),
                                                          tr("Marva project (*.marva)"));
    EmitFileNameAndAccept(projectFileName);
}

void StartDialog::createEmptyProject()
{
    EmitFileNameAndAccept("");
}
