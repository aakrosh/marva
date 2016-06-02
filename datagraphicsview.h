#ifndef DATAGRAPHICSVIEW_H
#define DATAGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QPrinter>
#include <QMenu>

class TaxDataProvider;
class BaseTaxNode;

class DataGraphicsView : public QGraphicsView
{
    Q_OBJECT
    QAction *printAction;
    QAction *screenshotAction;
protected:
    BaseTaxNode *curNode;
    QMenu popupMenu;
public:
    TaxDataProvider *taxDataProvider;
    bool persistant;
    DataGraphicsView(TaxDataProvider *_dataProvider, QWidget *parent = 0);
    virtual ~DataGraphicsView(){}
    virtual void setCurrentNode(BaseTaxNode *);
    inline BaseTaxNode *currentNode() { return curNode; }
    virtual void toJson(QJsonObject &) const {}
    virtual void fromJson(QJsonObject &) {}
    static DataGraphicsView *createViewByType(QWidget *parent, QString &type);
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

};

#endif // DATAGRAPHICSVIEW_H
