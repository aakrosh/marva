#include "main_window.h"
#include "ui_main_window.h"
#include "tree_loader_thread.h"
#include "map_loader_thread.h"
#include "graph_node.h"
#include "ui_components/taxlistwidget.h"
#include "ui_components/leftpanel.h"
#include "ui_components/currenttaxnodedetails.h"
#include "ui_components/labeleddoublespinbox.h"
#include "taxnodesignalsender.h"
#include "taxdataprovider.h"

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
void MainWindow::addGraphView(GraphView *gv, QString label)
{
    ui->tabWidget->addTab(gv, label);
    ui->tabWidget->setCurrentWidget(gv);
}

//=========================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    activeGraphView(NULL),
    taxonomyTreeView(NULL)
{
  mainWindow = this;
  generateDefaultNodes();
  globalTaxDataProvider = new GlobalTaxMapDataProvider(this, &taxMap);
  leftPanel = new LeftPanel(this);
  taxListWidget = leftPanel->taxList();
  taxListWidget->setTaxDataProvider(globalTaxDataProvider);

  connect(globalTaxDataProvider, SIGNAL(dataChanged()), taxListWidget, SLOT(refresh()));

  TreeLoaderThread *tlThread = new TreeLoaderThread(this, globalTaxDataProvider, true);
  connect(tlThread, SIGNAL(resultReady(void *)), globalTaxDataProvider, SLOT(onTreeLoaded()));
  connect(tlThread, SIGNAL(resultReady(void *)), this, SLOT(treeIsLoaded(void *)));
  connect(tlThread, SIGNAL(finished()), tlThread, SLOT(deleteLater()));
  tlThread->start();

  ui->setupUi(this);
  ui->taxListDockWidget->setWidget(leftPanel);

  readsSB = new LabeledDoubleSpinBox(NULL);
  this->ui->toolBar->addWidget(readsSB);
  readsSB->setLabel("Min reads");
  readsSB->setValue(0);
  readsSB->setToolTip("Minimum reads threshold to show");
  readsSB->setVisible(false);

  connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabCnaged(int)));

  openTaxonomyTreeView();
  connect(globalTaxDataProvider, SIGNAL(dataChanged()), taxonomyTreeView, SLOT(onTreeChanged()));

  statusList = new StatusListPanel(this);
  statusList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  centralWidget()->layout()->addWidget(statusList);
  statusList->setMaximumHeight(0);
  ui->action_Tab_separated_BLAST_file->setEnabled(false);
  connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeGraphView(int)));

  activeGraphView->setFocus();

  TaxNodeSignalSender *tnss = getTaxNodeSignalSender(NULL);
  connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), activeGraphView, SLOT(onCurrentNodeChanged(BaseTaxNode*)));
  connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), taxListWidget, SLOT(onCurrentTaxChanged(BaseTaxNode*)));
  connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), leftPanel->curNodeDetails(), SLOT(onCurrentNodeChanged(BaseTaxNode*)));
  connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), taxListWidget, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
  connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), activeGraphView, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
  connect(tnss, SIGNAL(bigChangesHappened()), taxListWidget, SLOT(resetView()));
  connect(tnss, SIGNAL(bigChangesHappened()), activeGraphView, SLOT(reset()));

  activeGraphView->setCurrentNode(taxTree);

  connectGraphView(NULL, activeGraphView);

  connect(ui->actionTaxonomyTree, SIGNAL(triggered(bool)), this, SLOT(openTaxonomyTreeView()));
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
void MainWindow::connectGraphView(GraphView *oldGV, GraphView *newGV)
{
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(NULL);
    if ( oldGV != NULL )
    {
        tnss->disconnect(oldGV);
        readsSB->disconnect(oldGV);
    }
    connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), newGV, SLOT(onCurrentNodeChanged(BaseTaxNode*)));
    connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), newGV, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
    connect(tnss, SIGNAL(bigChangesHappened()), newGV, SLOT(reset()));

    connect(readsSB, SIGNAL(valueChanged(quint32,quint32)), newGV, SLOT(onReadsThresholdChanged(quint32,quint32)));
}

//=========================================================================
void MainWindow::open_tab_blast_file()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open tab-separated BLAST file"));
    if ( fileName.isEmpty() )
        return;
    GraphView *blastView = new GraphView(this, NULL);
    BlastTaxDataProvider *blastTaxDataProvider = new BlastTaxDataProvider(blastView);
    BlastDataTreeLoader *bdtl = new BlastDataTreeLoader(this, fileName, blastTaxDataProvider, tabular);

    blastView->dirtyList.clear();
    blastView->scene()->clear();
    blastView->root = NULL;
    blastView->taxDataProvider = blastTaxDataProvider;
    taxListWidget->setTaxDataProvider(blastTaxDataProvider);
    taxListWidget->reset();
    ui->taxListDockWidget->setVisible(true);
    addGraphView(blastView, QFileInfo(fileName).fileName());
    readsSB->setVisible(true);
    readsSB->setReadOnly(true);

    connect(bdtl, SIGNAL(progress(void *)), this, SLOT(blastLoadingProgress(void *)));
    connect(bdtl, SIGNAL(resultReady(void *)), taxListWidget, SLOT(refreshAll()));
    connect(bdtl, SIGNAL(resultReady(void *)), this, SLOT(blastIsLoaded(void *)));
    connect(bdtl, SIGNAL(resultReady(void *)), blastView, SLOT(blastIsLoaded(void *)));
    connect(bdtl, SIGNAL(finished()), bdtl, SLOT(deleteLater()));

    bdtl->start();
}

//=========================================================================
GraphView *MainWindow::openTaxonomyTreeView()
{
    if ( taxonomyTreeView == NULL )
    {
        taxonomyTreeView = new GraphView(this, taxTree);
        taxonomyTreeView->persistant = true;
        taxonomyTreeView->taxDataProvider = globalTaxDataProvider;
    }
    setActiveGraphView(taxonomyTreeView);

    int i = ui->tabWidget->indexOf(taxonomyTreeView);
    if ( i >= 0 )
        ui->tabWidget->setCurrentIndex(i);
    else
        addGraphView(taxonomyTreeView, "Taxonomy tree");
    return taxonomyTreeView;
}

//=========================================================================
void MainWindow::treeIsLoaded(void *obj)
{
    TaxNode *tree = (TaxNode *)obj;
    qDebug() << "Tree Loading is finished";
    taxTree->mergeWith(tree, activeGraphView);
    ui->action_Tab_separated_BLAST_file->setEnabled(true);

    disconnect(globalTaxDataProvider, SIGNAL(dataChanged()), taxonomyTreeView, SLOT(onTreeChanged()));
    connect(globalTaxDataProvider, SIGNAL(dataChanged()), taxonomyTreeView, SLOT(onNodeNamesChanged()));

    MapLoaderThread *mlThread = new MapLoaderThread(this, true, globalTaxDataProvider);

    connect(mlThread, SIGNAL(progress(void *)), this, SLOT(updateLoadedNames()));
    connect(mlThread, SIGNAL(progress(void *)), taxListWidget, SLOT(refreshValues()));
    connect(mlThread, SIGNAL(progress(void*)), globalTaxDataProvider, SLOT(onMapChanged()));

    connect(mlThread, SIGNAL(resultReady(void*)), globalTaxDataProvider, SLOT(onMapChanged()));
    connect(mlThread, SIGNAL(resultReady(void *)), taxListWidget, SLOT(refresh()));

    connect(mlThread, SIGNAL(finished()), mlThread, SLOT(deleteLater()));
    mlThread->start();

}

//=========================================================================
void MainWindow::updateLoadedNames()
{
    activeGraphView->updateDirtyNodes(DIRTY_NAME);
    BaseTaxNode *node = activeGraphView->currentNode();
    leftPanel->curNodeDetails()->onCurrentNodeChanged(node);
}

//=========================================================================
void MainWindow::mapIsLoaded()
{
    qDebug() << "Map Loading is finished";
    // Handle end of mapLoading
    activeGraphView->updateDirtyNodes(DIRTY_NAME);
}

//=========================================================================
void MainWindow::blastLoadingProgress(void *)
{
    readsSB->setMaxValue(activeGraphView->taxDataProvider->getMaxReads());
    readsSB->setValue(0);
}

//=========================================================================
void MainWindow::closeGraphView(int i)
{
    QWidget *view = ui->tabWidget->widget(i);
    view->close();
    ui->tabWidget->removeTab(i);
}

//=========================================================================
void MainWindow::setActiveGraphView(GraphView *newGV)
{
    if ( activeGraphView == newGV )
        return;
    GraphView *oldGV = activeGraphView;
    activeGraphView = newGV;
    connectGraphView(oldGV, newGV);
    taxListWidget->setTaxDataProvider(newGV->taxDataProvider);
    emit activeGraphViewChanged(oldGV, newGV);
}

//=========================================================================
void MainWindow::onCurrentTabCnaged(int)
{
    GraphView *gv = dynamic_cast<GraphView *>(ui->tabWidget->currentWidget());
    if ( gv != NULL )
        setActiveGraphView(gv);
}

//=========================================================================
void MainWindow::blastIsLoaded(void *)
{
    blastLoadingProgress(NULL);
    readsSB->setReadOnly(false);
}
