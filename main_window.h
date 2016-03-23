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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    GraphView *activeGraphView;
    StatusListPanel *statusList;
protected:
    virtual void closeEvent(QCloseEvent *event);
private:
    Ui::MainWindow *ui;
    QList<LoaderThread*> activeLoaderThreads;
    LeftPanel *leftPanel;
    TaxListWidget *taxListWidget;
private slots:
    void mapIsLoaded();
    void updateLoadedNames();
    void open_tab_blast_file();
    void treeIsLoaded(void *obj);
};
extern MainWindow *mainWindow;

#endif
