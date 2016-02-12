#include "main_window.h"
#include "ui_main_window.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  GraphView *gwidget = new GraphView;
  setCentralWidget(gwidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open_tab_blast_file()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open tab-separated BLAST file"));
    if ( fileName.isEmpty() )
        return;
    blastData = new BlastData(tabular, fileName);
}
