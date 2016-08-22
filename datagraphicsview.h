#ifndef DATAGRAPHICSVIEW_H
#define DATAGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QPrinter>
#include <QMenu>
#include <QFile>

class TaxDataProvider;
class BaseTaxNode;

#define DGF_NONE        0x0
#define DGF_READS       0x1
#define DGF_BUBBLES     0x2

class GraphicsViewConfig
{
public:
    virtual ~GraphicsViewConfig() {}
};

class DataGraphicsView : public QGraphicsView
{
    Q_OBJECT
    QAction *printAction;
    QAction *screenshotAction;
protected:
//    BaseTaxNode *curNode;
    QMenu popupMenu;
    quint64 flags;
public:
    GraphicsViewConfig *config;
    TaxDataProvider *taxDataProvider;
    bool persistant;
    DataGraphicsView(TaxDataProvider *_dataProvider, QWidget *parent = 0);
    virtual ~DataGraphicsView();
    virtual void setCurrentNode(BaseTaxNode *);
    BaseTaxNode *currentNode();
    virtual void toJson(QJsonObject &) const {}
    virtual void fromJson(QJsonObject &) {}
    static DataGraphicsView *createViewByType(QWidget *parent, QString &type);
    inline quint64 getFlags() { return flags; }
    virtual void serialize(QFile &file);
    virtual void deserialize(QFile & /*file*/, int /*version*/){}
protected slots:
    virtual void onCurrentNodeChanged(BaseTaxNode *) {}
    virtual void print();
    virtual void makeScreenshot();
    void renderToPrinter(QPrinter *);
    void renderToPainter(QPainter *);
    virtual void showContextMenu(const QPoint&);
public slots:
    virtual void onNodeVisibilityChanged(BaseTaxNode*, bool) {}
    virtual void reset() {}
    virtual void onReadsThresholdChanged(quint32 /*oldT*/, quint32 /*newT*/) {}
    virtual void onBubbleSizeChanged(quint32 /*oldS*/, quint32 /*newS*/) {}
    virtual void onColorChanged(BaseTaxNode *) {}

};

#endif // DATAGRAPHICSVIEW_H
