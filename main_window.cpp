#include "main_window.h"
#include "ui_main_window.h"
#include "tree_loader_thread.h"
#include "map_loader_thread.h"
#include "graph_node.h"
#include "ui_components/taxlistwidget.h"

#include <QFileDialog>
#include <QDebug>

TaxMap taxMap;
TaxNode *taxTree;
MainWindow *mainWindow;

//=========================================================================
void generateDefaultNodes()
{
    // To improve the startup speed, first create hardcoded default nodes, they will be updated then the whole tree will be loaded
    taxTree = new TaxNode(1);
    taxMap.insert(1, taxTree);
    taxMap.setName(1, "root");
    taxTree->setCollapsed(false, false);

    TaxNode *n131567 = (TaxNode *)taxTree->addChildById(131567);
    taxMap.insert(131567, n131567);
    taxMap.setName(131567, "cellular organisms");
    n131567->setCollapsed(false, false);

    TaxNode *n2 = (TaxNode *)n131567->addChildById(2);
    taxMap.insert(2, n2);
    taxMap.setName(2, "Bacteria");

    taxMap.insert(201174, (TaxNode *)n2->addChildById(201174));
    taxMap.setName(201174, "Actinobacteria <phylum>");

    taxMap.insert(2157, (TaxNode *)n131567->addChildById(2157));
    taxMap.setName(2157, "Archaea");

    taxMap.insert(2759, (TaxNode *)n131567->addChildById(2759));
    taxMap.setName(2759, "Eukaryota");

    TaxNode *n10239 = (TaxNode *)taxTree->addChildById(10239);
    taxMap.insert(10239, n10239);
    taxMap.setName(10239, "Viruses");

    taxMap.insert(39759, (TaxNode *)n10239->addChildById(39759));
    taxMap.setName(39759, "Deltavirus");

    TaxNode *n12884 = (TaxNode *)taxTree->addChildById(12884);
    taxMap.insert(12884, n12884);
    taxMap.setName(12884, "Viroids");

    taxMap.insert(185752, (TaxNode *)n12884->addChildById(185752));
    taxMap.setName(185752, "Avsunviroidae");

    taxMap.insert(12908, (TaxNode *)taxTree->addChildById(12908));
    taxMap.setName(12908, "unclassified sequences");

    taxMap.insert(28384, (TaxNode *)taxTree->addChildById(28384));
    taxMap.setName(28384, "other sequences");
}

//=========================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  mainWindow = this;
  generateDefaultNodes();
  TaxListWidget *tlw = new TaxListWidget(this);

  TreeLoaderThread *tlThread = new TreeLoaderThread(this, &taxMap, true);
  connect(tlThread, SIGNAL(resultReady(void *)), this, SLOT(treeIsLoaded(void *)));
  connect(tlThread, SIGNAL(resultReady(void *)), tlw, SLOT(refresh()));
  connect(tlThread, SIGNAL(finished()), tlThread, SLOT(deleteLater()));
  tlThread->start();

  ui->setupUi(this);
  ui->taxListDockWidget->setWidget(tlw);
  activeGraphView = new GraphView(this, taxTree);
  centralWidget()->layout()->addWidget(activeGraphView);
  statusList = new StatusListPanel(this);
  statusList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  centralWidget()->layout()->addWidget(statusList);
  statusList->setMaximumHeight(0);

  connect(tlw, SIGNAL(currentTaxChanged(BaseTaxNode*)), activeGraphView, SLOT(onCurrentNodeChanged(BaseTaxNode*)));
  connect(activeGraphView, SIGNAL(currentNodeChanged(BaseTaxNode*)), tlw, SLOT(onCurrentTaxChanged(BaseTaxNode*)));
  activeGraphView->setFocus();
}

//=========================================================================
MainWindow::~MainWindow()
{
    delete ui;
}

//=========================================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    LoaderThread::StopRunningThreads();
    QMainWindow::closeEvent(event);
}

//=========================================================================
void MainWindow::open_tab_blast_file()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open tab-separated BLAST file"));
    if ( fileName.isEmpty() )
        return;
    activeGraphView->dirtyList.clear();
    activeGraphView->scene()->clear();
    activeGraphView->root = NULL;
    BlastDataTreeLoader *bdtl = new BlastDataTreeLoader(this, fileName, tabular);
    connect(bdtl, SIGNAL(progress(void *)), activeGraphView, SLOT(blastLoadingProgress(void *)));
    connect(bdtl, SIGNAL(progress(void *)), ui->taxListDockWidget->widget(), SLOT(refresh()));
    connect(bdtl, SIGNAL(resultReady(void *)), activeGraphView, SLOT(blastIsLoaded(void *)));
    connect(bdtl, SIGNAL(resultReady(void *)), ui->taxListDockWidget->widget(), SLOT(refresh()));
    connect(bdtl, SIGNAL(finished()), bdtl, SLOT(deleteLater()));
    bdtl->start();
}

//=========================================================================
void MainWindow::treeIsLoaded(void *obj)
{
    TaxNode *tree = (TaxNode *)obj;
    qDebug() << "Tree Loading is finished";
    taxTree->mergeWith(tree, activeGraphView);

    MapLoaderThread *mlThread = new MapLoaderThread(this, true, activeGraphView, &taxMap);
    connect(mlThread, SIGNAL(progress(void *)), this, SLOT(updateLoadedNames()));
    connect(mlThread, SIGNAL(progress(void *)), ui->taxListDockWidget->widget(), SLOT(refreshValues()));
    connect(mlThread, SIGNAL(resultReady(void *)), this, SLOT(mapIsLoaded()));
    connect(mlThread, SIGNAL(resultReady(void *)), ui->taxListDockWidget->widget(), SLOT(refresh()));
    connect(mlThread, SIGNAL(finished()), mlThread, SLOT(deleteLater()));
    mlThread->start();

    activeGraphView->updateDirtyNodes(DIRTY_CHILD);
    activeGraphView->createMissedGraphNodes();
}

//=========================================================================
void MainWindow::updateLoadedNames()
{
    activeGraphView->updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void MainWindow::mapIsLoaded()
{
    qDebug() << "Map Loading is finished";
    // Handle end of mapLoading
    activeGraphView->updateDirtyNodes(DIRTY_NAME);
}
