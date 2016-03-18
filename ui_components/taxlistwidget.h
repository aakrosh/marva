#ifndef TAXLISTWIDGET_H
#define TAXLISTWIDGET_H

#include "tax_map.h"

#include <QWidget>
#include <QAbstractTableModel>
#include <QGraphicsView>

#include <QDebug>
#include <QList>

class TaxNode;
class BaseTaxNode;

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

extern TaxMap taxMap;
class GlobalTaxMapDataProvider : public TaxDataProvider
{
    QList<TaxNode *> taxnodes;
    QList<int> ids;
public:
    GlobalTaxMapDataProvider();
    virtual quint32 count();
    virtual qint32 id(quint32 index);
    virtual BaseTaxNode *taxNode(quint32 index);
    virtual quint32 reads(quint32 /*index*/);
    virtual quint32 indexOf(qint32 id);
    virtual void updateCache(bool values_only);
};

class TaxListTableModel : public QAbstractItemModel
{
private:
  //TaxMap *listTaxMap;
  TaxDataProvider *taxDataProvider;
  //QList<TaxNode *> taxnodes;
  //QList<int> ids;

public:
  TaxListTableModel(QObject *parent);
  ~TaxListTableModel()
  {
      delete taxDataProvider;
  }
  QModelIndex index(int row, int column, const QModelIndex &/*parent*/ = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const ;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  void sort(int column, Qt::SortOrder order);
  void emitDataChangedSignal(const QModelIndex &start, const QModelIndex &end);
//  void setTaxMap(TaxMap *taxMap, bool refreshValuesOnly);
  void setTaxDataProvider(TaxDataProvider *td, bool refreshValuesOnly);
  Qt::ItemFlags flags(const QModelIndex &index) const;
  void clearCache();

  friend class TaxListWidget;
};


class TaxListWidget : public QWidget
{
    Q_OBJECT
    TaxListTableModel *model;
public:
    explicit TaxListWidget(QWidget *parent = 0);
    ~TaxListWidget();
//    inline void setTaxMap(TaxMap *map) { model->listTaxMap = map; }
    inline void setTaxDataProvider(TaxDataProvider *tdp) { model->taxDataProvider = tdp; }
    void reset();

private:
    Ui::TaxListWidget *ui;
    int oldRowCount;

signals:
    void currentTaxChanged(BaseTaxNode *);

private slots:
    void refresh();
    void refreshAll();
    void refreshValues();
    void taxChanged(QModelIndex, QModelIndex);
    void onCurrentTaxChanged(BaseTaxNode *);
};

#endif // TAXLISTWIDGET_H
