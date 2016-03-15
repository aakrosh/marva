#ifndef TAXLISTWIDGET_H
#define TAXLISTWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QGraphicsView>

#include <QDebug>
#include <QList>
class TaxMap;
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

class TaxListTableModel : public QAbstractItemModel
{
private:
  TaxMap *listTaxMap;
  QList<TaxNode *> taxnodes;
  QList<int> ids;

public:
  TaxListTableModel(QObject *parent);
  QModelIndex index(int row, int column, const QModelIndex &/*parent*/ = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const ;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  void sort(int column, Qt::SortOrder order);
  void emitDataChangedSignal(const QModelIndex &start, const QModelIndex &end);
  void setTaxMap(TaxMap *taxMap, bool refreshValuesOnly);
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
    inline void setTaxMap(TaxMap *map) { model->listTaxMap = map; }
    void reset();

private:
    Ui::TaxListWidget *ui;

signals:
    void currentTaxChanged(BaseTaxNode *);

private slots:
    void refresh();
    void refreshValues();
    void taxChanged(QModelIndex, QModelIndex);
    void onCurrentTaxChanged(BaseTaxNode *);
};

#endif // TAXLISTWIDGET_H
