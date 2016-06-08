#ifndef DATASOURCESMODEL_H
#define DATASOURCESMODEL_H

#include <QAbstractItemModel>

class BlastTaxDataProviders;

class DataSourcesModel : public QAbstractItemModel
{
  BlastTaxDataProviders *dataProviders;
public:
  DataSourcesModel(QObject *parent, BlastTaxDataProviders *providers);
  ~DataSourcesModel();
  QModelIndex index(int row, int column, const QModelIndex &/*parent*/ = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const ;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role);
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // DATASOURCESMODEL_H
