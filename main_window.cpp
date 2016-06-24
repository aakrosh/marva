#include "main_window.h"
#include "ui_main_window.h"
#include "tree_loader_thread.h"
#include "map_loader_thread.h"
#include "graph_node.h"
#include "taxnodesignalsender.h"
#include "taxdataprovider.h"
#include "colors.h"
#include "config.h"
#include "blastfileloader.h"

#include "ui_components/taxlistwidget.h"
#include "ui_components/leftpanel.h"
#include "ui_components/currenttaxnodedetails.h"
#include "ui_components/labeleddoublespinbox.h"
#include "ui_components/start_dialog.h"
#include "ui_components/configurationdialog.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>

TaxMap taxMap;
MainWindow *mainWindow;

//=========================================================================
void generateDefaultNodes()
{
    // To improve the startup speed, first create hardcoded default nodes, they will be updated then the whole tree will be loaded
    globalTaxDataProvider->taxTree = new TaxNode(1);
    taxMap.insert(1, globalTaxDataProvider->taxTree);
    taxMap.setName(1, "root");
    globalTaxDataProvider->taxTree->setCollapsed(false, false);

    TaxNode *n131567 = (TaxNode *)globalTaxDataProvider->taxTree->addChildById(131567);
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

    TaxNode *n10239 = (TaxNode *)globalTaxDataProvider->taxTree->addChildById(10239);
    taxMap.insert(10239, n10239);
    taxMap.setName(10239, "Viruses");

    taxMap.insert(39759, (TaxNode *)n10239->addChildById(39759));
    taxMap.setName(39759, "Deltavirus");

    TaxNode *n12884 = (TaxNode *)globalTaxDataProvider->taxTree->addChildById(12884);
    taxMap.insert(12884, n12884);
    taxMap.setName(12884, "Viroids");

    taxMap.insert(185752, (TaxNode *)n12884->addChildById(185752));
    taxMap.setName(185752, "Avsunviroidae");

    taxMap.insert(12908, (TaxNode *)globalTaxDataProvider->taxTree->addChildById(12908));
    taxMap.setName(12908, "unclassified sequences");

    taxMap.insert(28384, (TaxNode *)globalTaxDataProvider->taxTree->addChildById(28384));
    taxMap.setName(28384, "other sequences");
}

//=========================================================================
void MainWindow::addGraphView(QWidget *gv, QString label)
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
    history = AbstractConfigFileFactory<History>::create(this);
    colors = AbstractConfigFileFactory<Colors>::create(this);
    configuration = AbstractConfigFileFactory<Config>::create(this);

    globalTaxDataProvider = new GlobalTaxMapDataProvider(this, &taxMap);
    mainWindow = this;
    generateDefaultNodes();
    leftPanel = new LeftPanel(this);
    taxListWidget = leftPanel->taxList();
    leftPanel->setTaxDataProvider(globalTaxDataProvider);

    connect(globalTaxDataProvider, SIGNAL(dataChanged()), taxListWidget, SLOT(refresh()));
    TreeLoaderThread *tlThread = new TreeLoaderThread(this, globalTaxDataProvider, true);
    connect(tlThread, SIGNAL(resultReady(void *)), globalTaxDataProvider, SLOT(onTreeLoaded()));
    connect(tlThread, SIGNAL(resultReady(void *)), this, SLOT(treeIsLoaded(void *)));
    connect(tlThread, SIGNAL(resultReady(void *)), taxListWidget, SLOT(reset()));
    connect(tlThread, SIGNAL(finished()), tlThread, SLOT(deleteLater()));
    tlThread->start();

    ui->setupUi(this);
    ui->taxListDockWidget->setWidget(leftPanel);

    readsSlider = new SliderWithEdit(this);
    readsSlider->setLabel("Threshold");
    readSliderAction = ui->toolBar->addWidget(readsSlider);
    readsSlider->setToolTip("Minimum reads threshold to show");
    readsSlider->setVisible(false);

    bubbleSlider = new SliderWithEdit(this);
    bubbleSlider->setLabel("Bubble size");
    bubbleSlider->setMaxValue(200);
    bubbleSlider->setMinValue(10);
    bubbleSliderAction = ui->toolBar->addWidget(bubbleSlider);
    bubbleSlider->setToolTip("Size of biggest bubble");
    bubbleSlider->setVisible(false);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabCnaged(int)));

    TreeGraphView *taxTreeView = openTaxonomyTreeView();
    connect(globalTaxDataProvider, SIGNAL(dataChanged()), taxonomyTreeView, SLOT(onTreeChanged()));

    statusList = new StatusListPanel(this);
    statusList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    centralWidget()->layout()->addWidget(statusList);
    statusList->setMaximumHeight(0);
    ui->action_Tab_separated_BLAST_file->setEnabled(false);
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeGraphView(int)));

    taxTreeView->setFocus();

    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(NULL);
    connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), taxTreeView, SLOT(onCurrentNodeChanged(BaseTaxNode*)));
    connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), taxListWidget, SLOT(onCurrentTaxChanged(BaseTaxNode*)));
    connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), leftPanel->curNodeDetails(), SLOT(onCurrentNodeChanged(BaseTaxNode*)));
    connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), taxListWidget, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
    connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), taxTreeView, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
    connect(tnss, SIGNAL(bigChangesHappened()), taxListWidget, SLOT(resetView()));
    connect(tnss, SIGNAL(bigChangesHappened()), taxTreeView, SLOT(reset()));

    taxTreeView->setCurrentNode(globalTaxDataProvider->taxTree);

    connectGraphView(NULL, taxTreeView);

    connect(ui->actionTaxonomyTree, SIGNAL(triggered(bool)), this, SLOT(openTaxonomyTreeView()));
    connect(ui->actionCreateChart, SIGNAL(triggered(bool)), this, SLOT(createChartView()));

    StartDialog *startDialog = new StartDialog(this);
    connect(startDialog, SIGNAL(fileChoosen(QString)), this, SLOT(openProject(QString)));
    connect(startDialog, SIGNAL(accepted()), startDialog, SLOT(deleteLater()));
    startDialog->open();
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
void MainWindow::connectGraphView(DataGraphicsView *oldGV, DataGraphicsView *newGV)
{
    TaxNodeSignalSender *tnss = getTaxNodeSignalSender(NULL);
    if ( oldGV != NULL )
    {
        tnss->disconnect(oldGV);
        readsSlider->disconnect(oldGV);
        bubbleSlider->disconnect(oldGV);
    }
    connect(tnss, SIGNAL(makeCurrent(BaseTaxNode*)), newGV, SLOT(onCurrentNodeChanged(BaseTaxNode*)));
    connect(tnss, SIGNAL(visibilityChanged(BaseTaxNode*,bool)), newGV, SLOT(onNodeVisibilityChanged(BaseTaxNode*,bool)));
    connect(tnss, SIGNAL(bigChangesHappened()), newGV, SLOT(reset()));
    connect(tnss, SIGNAL(bigChangesHappened()), newGV, SLOT(reset()));
    connect(tnss, SIGNAL(colorChanged(BaseTaxNode*)), newGV, SLOT(onColorChanged(BaseTaxNode*)));

    connect(newGV, SIGNAL(destroyed(QObject*)), this, SLOT(activeGraphViewDestroyed()));

    connect(readsSlider, SIGNAL(valueChanged(quint32, quint32)), newGV, SLOT(onReadsThresholdChanged(quint32,quint32)));
    connect(bubbleSlider, SIGNAL(valueChanged(quint32, quint32)), newGV, SLOT(onBubbleSizeChanged(quint32,quint32)));
}

//=========================================================================
void MainWindow::open_tab_blast_files()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList fileNames;
    if ( dialog.exec() )
        fileNames = dialog.selectedFiles();
    if ( fileNames.isEmpty() )
        return;
    foreach (QString fileName, fileNames)
        open_tab_blast_file(fileName);
}

//=========================================================================
void MainWindow::openProject(QString fileName)
{
    if ( fileName.isEmpty() )
        return;
    treeLoaderMutex.lock();     // No actual locking is needed. Just wait till the tree is loaded
    treeLoaderMutex.unlock();
    QFile loadFile(fileName);
    if ( !loadFile.open(QIODevice::ReadOnly) )
    {
        qWarning("Couldn't open project file.");
        return;
    }
    QString status = QString("Loading project file %1").arg(loadFile.fileName());
    QListWidgetItem *statusListItem = statusList->AddItem(status);
    QCoreApplication::processEvents();
    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc = QJsonDocument::fromJson(saveData);
    QJsonObject jobj = loadDoc.object();
    fromJson(jobj);
    loadFile.close();
    statusList->RemoveItem(statusListItem);
    history->addProject(fileName);
}

//=========================================================================
void MainWindow::open_project()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open project"),
                                                          QString(),
                                                          tr("Marva project (*.marva)"));
    if ( fileName.isEmpty() )
        return;
    closeAllViews();
    openProject(fileName);
}

//=========================================================================
void MainWindow::save_project()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save project"),
                                                          QString(),
                                                          tr("Marva project (*.marva)"));
    if ( fileName.isEmpty() )
        return;
    QFile saveFile(fileName);

    if ( !saveFile.open(QIODevice::WriteOnly ))
        qWarning("Couldn't open project file for writing.");

    QJsonObject saveObject;
    toJson(saveObject);
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson(QJsonDocument::Compact));
    saveFile.close();
    history->addProject(fileName);
}

//=========================================================================
void MainWindow::close_project()
{
    closeAllViews();
}

//=========================================================================
void MainWindow::toJson(QJsonObject &json) const
{
    blastTaxDataProviders.toJson(json);
    QJsonArray jViewArr;
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
    {
        DataGraphicsView *dgv = (DataGraphicsView *)ui->tabWidget->widget(i);
        if ( dgv != NULL && !dgv->persistant )
        {
            QJsonObject jview;
            dgv->toJson(jview);
            jViewArr.append(jview);
        }
    }
    json["views"] = jViewArr;
}

//=========================================================================
void MainWindow::fromJson(QJsonObject &json)
{
    try
    {
        QCoreApplication::processEvents();
        blastTaxDataProviders.clear();
        blastTaxDataProviders.fromJson(json);
        QCoreApplication::processEvents();
        if ( blastTaxDataProviders.size() == 0 )
        {
            QMessageBox::warning(0, "Cannot open project", QString("UNo data providers are found in the project file"));
            return;
        }
        QJsonArray jViewArr = json["views"].toArray();
        for ( int i = 0; i < jViewArr.size(); i++ )
        {
            QCoreApplication::processEvents();
            QJsonObject jView = jViewArr[i].toObject();;
            QString type = jView["Type"].toString();
            DataGraphicsView *dgv = DataGraphicsView::createViewByType(this, type);
            if ( dgv == NULL )
            {
                QMessageBox::warning(0, "Cannot create view", QString("Unknown view type %1").arg(type));
                continue;
            }
            dgv->fromJson(jView);
            addGraphView(dgv, dgv->taxDataProvider->name);
            leftPanel->setTaxDataProvider(dgv->taxDataProvider);
        }
    }
    catch (...)
    {
        QMessageBox::warning(this, "Error occured", "Cannot load project");
    }
}

//=========================================================================
void MainWindow::closeAllViews()
{
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
    {
        DataGraphicsView *dgv = (DataGraphicsView *)ui->tabWidget->widget(i);
        if ( dgv != NULL && !dgv->persistant )
            dgv->close();
    }
}

//=========================================================================
void MainWindow::open_tab_blast_file(QString fileName)
{
    BlastTaxDataProvider *blastTaxDataProvider = new BlastTaxDataProvider(NULL);
    BlastGraphView *blastView = new BlastGraphView(blastTaxDataProvider, this, NULL);
    BlastFileLoader *bdtl = new BlastFileLoader(this, fileName, blastTaxDataProvider, tabular);

    connect(blastView, SIGNAL(blast_view_closed()), bdtl, SLOT(stop_thread()));

    leftPanel->setTaxDataProvider(blastTaxDataProvider);
    ui->taxListDockWidget->setVisible(true);
    addGraphView(blastView, blastTaxDataProvider->name);

    //readsSlider->setEnabled(false);

    connect(bdtl, SIGNAL(progress(void *)), this, SLOT(blastLoadingProgress(void *)));
    connect(bdtl, SIGNAL(progress(void *)), blastView, SLOT(blastLoadingProgress(void *)));
    connect(bdtl, SIGNAL(progress(void *)), taxListWidget, SLOT(refresh()));
    connect(bdtl, SIGNAL(resultReady(void *)), taxListWidget, SLOT(refreshAll()));
    connect(bdtl, SIGNAL(resultReady(void *)), this, SLOT(blastIsLoaded(void *)));
    connect(bdtl, SIGNAL(resultReady(void *)), blastView, SLOT(blastIsLoaded(void *)));
    connect(bdtl, SIGNAL(finished()), bdtl, SLOT(deleteLater()));

    bdtl->start();
}

//=========================================================================
TreeGraphView *MainWindow::openTaxonomyTreeView()
{
    if ( taxonomyTreeView == NULL )
    {
        taxonomyTreeView = new TreeGraphView(this, globalTaxDataProvider->taxTree);
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
BlastTaxDataProviders *MainWindow::getAllBlastDataProviders()
{
    BlastTaxDataProviders *res = new BlastTaxDataProviders();
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
    {
        DataGraphicsView *qgv = (DataGraphicsView *)ui->tabWidget->widget(i);
        BlastGraphView *gv = dynamic_cast<BlastGraphView *>(qgv);
        if ( gv != NULL )
        {
            if ( gv->taxDataProvider->metaObject() == &BlastTaxDataProvider::staticMetaObject )
                res->addProvider((BlastTaxDataProvider *)gv->taxDataProvider);
        }
    }
    return res;
}

//=========================================================================
BubbleChartView *MainWindow::createChartView()
{
    BubbleChartView *cv = new BubbleChartView(getAllBlastDataProviders(), this);
    addGraphView(cv, "Chart");
    return cv;
}

//=========================================================================
void MainWindow::treeIsLoaded(void *obj)
{
    TaxNode *tree = (TaxNode *)obj;
    qDebug() << "Tree Loading is finished";
    globalTaxDataProvider->taxTree->mergeWith(tree, taxonomyTreeView);
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
void MainWindow::updateAllDirtyNames()
{
    for ( int i = 0; i < ui->tabWidget->count(); i++ )
    {
        DataGraphicsView *qgv = (DataGraphicsView *)ui->tabWidget->widget(i);
        TreeGraphView *gv = dynamic_cast<TreeGraphView *>(qgv);
        if ( gv != NULL )
            gv->updateDirtyNodes(DIRTY_NAME);
    }
}

//=========================================================================
void MainWindow::updateLoadedNames()
{
    updateAllDirtyNames();
    BaseTaxNode *node = activeGraphView->currentNode();
    leftPanel->curNodeDetails()->onCurrentNodeChanged(node);
}

//=========================================================================
void MainWindow::mapIsLoaded()
{
    updateAllDirtyNames();
}

//=========================================================================
void MainWindow::blastLoadingProgress(void *)
{
    readsSlider->setMaxValue(activeGraphView->taxDataProvider->getMaxReads());
    readsSlider->setValue(0);
}

//=========================================================================
void MainWindow::closeGraphView(int i)
{
    QWidget *view = ui->tabWidget->widget(i);
    view->close();
    ui->tabWidget->removeTab(i);
}

//=========================================================================
void MainWindow::setActiveGraphView(DataGraphicsView *newGV)
{
    if ( activeGraphView == newGV )
        return;
    DataGraphicsView *oldGV = activeGraphView;
    activeGraphView = newGV;
    connectGraphView(oldGV, newGV);
    leftPanel->setTaxDataProvider(newGV->taxDataProvider);
    BlastGraphView *bgv = dynamic_cast<BlastGraphView*>(newGV);
    if ( bgv != NULL )
    {
        readsSlider->setMaxValue(activeGraphView->taxDataProvider->getMaxReads());
        readsSlider->setValue(bgv->reads_threshold);
    }
    BubbledGraphViewConfig *bgv_config = dynamic_cast<BubbledGraphViewConfig*>(newGV->config);
    if ( bgv_config != NULL )
    {
        bubbleSlider->setValue(bgv_config->bubbleSize);
    }

    readSliderAction->setVisible((newGV->getFlags() & DGF_READS) != 0);
    bubbleSliderAction->setVisible((newGV->getFlags() & DGF_BUBBLES) != 0);
    emit activeGraphViewChanged(oldGV, newGV);
}

//=========================================================================
void MainWindow::onCurrentTabCnaged(int)
{
    DataGraphicsView *gv = dynamic_cast<DataGraphicsView *>(ui->tabWidget->currentWidget());
    if ( gv != NULL )
        setActiveGraphView(gv);
}

//=========================================================================
void MainWindow::activeGraphViewDestroyed()
{
}

//=========================================================================
void MainWindow::openOptionsDialog()
{
    ConfigurationDialog *cd = new ConfigurationDialog(this);
    cd->show();
}

//=========================================================================
void MainWindow::blastIsLoaded(void *)
{
    blastLoadingProgress(NULL);
}
