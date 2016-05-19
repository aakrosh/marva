#ifndef CHARTVIEW_H
#define CHARTVIEW_H
#include <QList>
#include "taxdataprovider.h"
#include "datagraphicsview.h"

#include <QMenu>

#define MARGIN 50
class BlastTaxNode;

typedef QList<BlastTaxNode *> BlastTaxNodes;

class IdBlastTaxNodesPair
{
public:
    quint32 id;
    BlastTaxNodes tax_nodes;
    bool checked;
    IdBlastTaxNodesPair(quint32 _id):id(_id),checked(true){}
    quint32 reads() const;
};

class ChartDataProvider : public TaxDataProvider
{
    Q_OBJECT
    BlastTaxDataProviders *providers;
    QList<IdBlastTaxNodesPair> data;
    quint32 maxreads;

public:
    ChartDataProvider(BlastTaxDataProviders *_providers, QObject *parent);
    virtual ~ChartDataProvider();
    virtual quint32 count();
    virtual QString text(quint32 index);
    virtual qint32 id(quint32 index);
    virtual quint32 reads(quint32 index);
    virtual quint32 readsById(quint32 id);
    virtual BaseTaxNode *taxNode(quint32 index);
    virtual void updateCache(bool values_only);
    virtual QColor color(int index);
    virtual void sort(int column, Qt::SortOrder order);
    virtual quint32 getMaxReads();
    bool contains(quint32 id);
    virtual quint32 indexOf(qint32 id);
    virtual QVariant checkState(int index);
    virtual void setCheckedState(int index, QVariant value);

signals:
    void taxVisibilityChanged(quint32 index);
    void cacheUpdated();
    friend class ChartView;
};

class ChartGraphNode;
class ChartView : public DataGraphicsView
{
    Q_OBJECT
    QGraphicsTextItem *header;
    QGraphicsRectItem *chartRectGI;
    QList<QGraphicsRectItem *> grid;
    QList<QGraphicsTextItem*> verticalLegend;
    QList<QGraphicsTextItem*> horizontalLegend;
public:
    ChartView(BlastTaxDataProviders *_dataProviders, QWidget *parent = 0);
    ~ChartView();
    void PrepareScene();
    QRectF chartRect;
    int ppl_count;
    quint32 maxNodeSize;
    ChartGraphNode *getGNode(BlastTaxNode *node);
    inline ChartDataProvider *dataProvider() { return (ChartDataProvider*)taxDataProvider; }

    void showChart();

    void setHeader(QString fileName);
    void setChartRectSize(int w, int h);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
private:
    QMenu popupMenu;
    QAction *propertiesAction;
    friend class ChartGraphNode;
    void goUp();
    void goDown();
protected slots:
    virtual void onCurrentNodeChanged(BaseTaxNode *);
    virtual void onTaxVisibilityChanged(quint32 index);
    virtual void onDataChanged();
    virtual void showContextMenu(const QPoint&);
    virtual void showPropertiesDialog();
public slots:
    virtual void onNodeVisibilityChanged(BaseTaxNode*, bool) {}
    virtual void reset() {}
    virtual void onReadsThresholdChanged(quint32 /*oldT*/, quint32 /*newT*/) {}
    virtual void changeMaxBubbleSize(int);
};


#endif // CHARTVIEW_H
