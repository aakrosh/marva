#include "start_dialog.h"
#include "ui_start_dialog.h"

#include <QFileDialog>

StartDialog::StartDialog(QWidget *parent) :
    QDialog(parent),
    projectFileName(""),
    ui(new Ui::StartDialog)
{
    ui->setupUi(this);
    ui->bLoadLast->setFocus();
}

StartDialog::~StartDialog()
{
    delete ui;
}

void StartDialog::openLastProject()
{
    // TODO: Get file name from history
    emit fileChoosen("");
    accept();
}

void StartDialog::openProjectFile()
{
    projectFileName = QFileDialog::getOpenFileName(this, tr("Open project"),
                                                          QString(),
                                                          tr("Marva project (*.marva)"));
    emit fileChoosen(projectFileName);
    accept();
}

void StartDialog::createEmptyProject()
{
    emit fileChoosen(projectFileName);
    accept();
}
