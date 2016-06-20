#ifndef TAXLISTWIDGET_H
#define TAXLISTWIDGET_H

#include "tax_map.h"

#include <QWidget>
#include <QAbstractTableModel>
#include <QGraphicsView>

#include <QDebug>

class TaxNode;
class BaseTaxNode;
class TaxDataProvider;

namespace Ui {
    class TaxListWidget;
}

class TaxListCacheData
{
public:
    QVariant text;
    QVariant id;
    QVariant reads;

    TaxListCacheData(QString t, qint32 i, quint32 r): text(t), id(i), reads(r){}
};

class TaxListCache: public QMap<int, TaxListCacheData *>
{
    int max_size;
    QList<int> ids;
public:
    TaxListCache(int msize = 100);
    void addRecord(int i, QString text, qint32 id, quint32 reads);
    virtual void clear();
};

class TaxListTableModel : public QAbstractItemModel
{
private:
  TaxDataProvider *taxDataProvider;

public:
  TaxListTableModel(QObject *parent);
  ~TaxListTableModel();
  QModelIndex index(int row, int column, const QModelIndex &/*parent*/ = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const ;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  void sort(int column, Qt::SortOrder order);
  void emitDataChangedSignal(const QModelIndex &start, const QModelIndex &end);
  void setTaxDataProvider(TaxDataProvider *td, bool refreshValuesOnly);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  void clearCache();
public:
  friend class TaxListWidget;
};


class TaxListWidget : public QWidget
{
    Q_OBJECT
    TaxListTableModel *model;
public:
    explicit TaxListWidget(QWidget *parent = 0);
    ~TaxListWidget();
    void setTaxDataProvider(TaxDataProvider *tdp);
    virtual bool eventFilter(QObject *object, QEvent *event);
private:
    Ui::TaxListWidget *ui;
    int oldRowCount;
signals:
    void currentTaxChanged(BaseTaxNode *);

private slots:
    void resetView();
    void refresh();
    void refreshAll();
    void refreshValues();
    void taxChanged(QModelIndex, QModelIndex);
    void onCurrentTaxChanged(BaseTaxNode *);
    void onColorChanged(BaseTaxNode *);
    void showContextMenu(const QPoint&);
    void changeCurrentTaxColor();
public slots:
    void reset();
    void onNodeVisibilityChanged(BaseTaxNode*,bool);
};

#endif // TAXLISTWIDGET_H
