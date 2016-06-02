#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "graphview.h"
#include "blast_data.h"
#include "ui_components/statuslistpanel.h"
#include "chartview.h"

namespace Ui {
class MainWindow;
}

extern TaxMap taxMap;
extern TaxNode *taxTree;
class MapLoaderThread;
class TaxListWidget;
class LeftPanel;
class LabeledDoubleSpinBox;
class GlobalTaxMapDataProvider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    StatusListPanel *statusList;
    LabeledDoubleSpinBox *readsSB;
    GlobalTaxMapDataProvider *globalTaxDataProvider;
    void addGraphView(QWidget *gv, QString label);

    void setActiveGraphView(DataGraphicsView *gv);

protected:
    virtual void closeEvent(QCloseEvent *event);
private:

    void connectGraphView(DataGraphicsView *oldGV, DataGraphicsView *newGV);
    void updateAllDirtyNames();
    Ui::MainWindow *ui;
    DataGraphicsView *activeGraphView;
    TreeGraphView *taxonomyTreeView;
    QList<LoaderThread*> activeLoaderThreads;
    LeftPanel *leftPanel;
    TaxListWidget *taxListWidget;


signals:
    activeGraphViewChanged(DataGraphicsView *oldGV, DataGraphicsView *newGV);
private slots:
    void mapIsLoaded();
    void updateLoadedNames();
    void open_tab_blast_file(QString fileName);
    void open_tab_blast_files();
    void open_project();
    void save_project();
    void close_project();
    void openProject(QString fileName);
    void toJson(QJsonObject &json) const;
    void fromJson(QJsonObject &json);
    void closeAllViews();
    TreeGraphView *openTaxonomyTreeView();
    BlastTaxDataProviders *getAllBlastDataProviders();
    ChartView *createChartView();
    void treeIsLoaded(void *obj);
    void blastIsLoaded(void *obj);
    void blastLoadingProgress(void *obj);
    void closeGraphView(int);
    void onCurrentTabCnaged(int);
    void activeGraphViewDestroyed();
};
extern MainWindow *mainWindow;

#endif
