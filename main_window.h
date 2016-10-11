#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>

#include "graphview.h"
#include "blast_data.h"
#include "ui_components/statuslistpanel.h"
#include "ui_components/sliderwithedit.h"
#include "ui_components/taxrankchooser.h"
#include "bubblechartview.h"
#include "history.h"

namespace Ui {
class MainWindow;
}

//extern TaxMap taxMap;
class MapLoaderThread;
class TaxListWidget;
class LeftPanel;
class LabeledDoubleSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QJsonArray jProviders;
    quint32 serializingProviders;
    QJsonObject big_json;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    StatusListPanel *statusList;
    SliderWithEdit *readsSlider;
    SliderWithEdit *bubbleSlider;
    TaxRankChooser *taxRankChooser;
    QAction *readSliderAction;
    QAction *bubbleSliderAction;
    QAction *taxRankChooserAction;

    void addGraphView(QWidget *gv, QString label);
    void setActiveGraphView(DataGraphicsView *gv);
    quint32 getThreshold();
    TaxRank getRank();
    void setRank(TaxRank rank);
    quint32 getBubbleSize();

    void serialize(QFile &saveFile);
    void deserialize(QFile &loadFile);

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
    History *history;

signals:
    void activeGraphViewChanged(DataGraphicsView *oldGV, DataGraphicsView *newGV);
    void allProvidersSerialized();
private slots:
    void mapIsLoaded();
    void updateLoadedNames();
    void open_tab_blast_file(QString fileName);
    void open_tab_blast_files();
    void open_project();
    void save_project();
    void save_current_project();
    void close_project();
    void openProject(QString fileName);
    void toJson();
    void fromJson(QJsonObject &json);
    void closeAllViews();
    TreeGraphView *openTaxonomyTreeView();
    BlastTaxDataProviders *getAllBlastDataProviders();
    BubbleChartView *createChartView();
    void treeIsLoaded(LoaderThread *loader);
    void blastIsLoaded(LoaderThread *loader);
    void blastLoadingProgress(LoaderThread *loader);
    void closeGraphView(int);
    void onCurrentTabCnaged(int);
    void activeGraphViewDestroyed();
    void openOptionsDialog();
    QString getOpenFileName(QString text, QString filters);
    void onProviderSerialized(ProvidersSerializationThread *);
    void finishSerialization();
    void onConfigChanged();        

    friend class DataGraphicsView;
};
extern MainWindow *mainWindow;

#endif
