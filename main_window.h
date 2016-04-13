#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "graphwidget.h"
#include "blast_data.h"
#include "ui_components/statuslistpanel.h"

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
    void addGraphView(GraphView *gv, QString label);

    void setActiveGraphView(GraphView *gv);

protected:
    virtual void closeEvent(QCloseEvent *event);
private:

    void connectGraphView(GraphView *oldGV, GraphView *newGV);
    Ui::MainWindow *ui;
    GraphView *activeGraphView;
    GraphView *taxonomyTreeView;
    QList<LoaderThread*> activeLoaderThreads;
    LeftPanel *leftPanel;
    TaxListWidget *taxListWidget;
signals:
    activeGraphViewChanged(GraphView *oldGV, GraphView *newGV);
private slots:
    void mapIsLoaded();
    void updateLoadedNames();
    void open_tab_blast_file();
    GraphView *openTaxonomyTreeView();
    void treeIsLoaded(void *obj);
    void blastIsLoaded(void *obj);
    void blastLoadingProgress(void *obj);
    void closeGraphView(int);
    void onCurrentTabCnaged(int);
};
extern MainWindow *mainWindow;

#endif
