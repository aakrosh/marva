#include "datasourcesmodel.h"
#include "taxdataprovider.h"


//=========================================================================
DataSourcesModel::DataSourcesModel(QObject *parent, BlastTaxDataProviders *providers) :
    QAbstractItemModel(parent),
    dataProviders(providers)
{

}

//=========================================================================
DataSourcesModel::~DataSourcesModel()
{

}

//=========================================================================
QModelIndex DataSourcesModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

//=========================================================================
QModelIndex DataSourcesModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

//=========================================================================
int DataSourcesModel::rowCount(const QModelIndex &) const
{
    return dataProviders->count();
}

//=========================================================================
int DataSourcesModel::columnCount(const QModelIndex &) const
{
    return 1;
}

//=========================================================================
QVariant DataSourcesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return dataProviders->at(index.row())->name;
    }
    else if (role == Qt::CheckStateRole  )
    {
        return dataProviders->isVisible(index.row()) ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

//=========================================================================
bool DataSourcesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( role == Qt::CheckStateRole && index.column() == 0 )
    {
        dataProviders->setVisible(index.row(), value == Qt::Checked);
        QVector<int> roles;
        roles.append(Qt::CheckStateRole);
        emit dataChanged(index, index, roles);
        return true;
    }
    return false;
}

//=========================================================================
QVariant DataSourcesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( orientation == Qt::Horizontal )
        {
            QStringList lst;
            lst << "Data sources";
            if ( section < lst.size() )
                return lst[section];
        }
    }
    return QVariant();
}

//=========================================================================
Qt::ItemFlags DataSourcesModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}


