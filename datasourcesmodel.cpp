#include "datasourcesmodel.h"
#include "taxdataprovider.h"

#include <QMimeData>
#include <QDataStream>

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
Qt::ItemFlags DataSourcesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    if (index.isValid())
        return Qt::ItemIsDragEnabled | flags;
    return flags;
}

//=========================================================================
QStringList DataSourcesModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}


//=========================================================================
QMimeData *DataSourcesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes)
    {
        if ( index.isValid() )
        {
//            QString text = data(index, Qt::DisplayRole).toString();
            stream << index.row();
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

//=========================================================================
bool DataSourcesModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.text.list"))
        return false;

    if (column > 0)
        return false;

    int beginRow;
    int maxRowCount = rowCount(QModelIndex());

    if ( row != -1 || row >= maxRowCount )
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = maxRowCount-1;

    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<int> movedProviderIndexes;

    while (!stream.atEnd())
    {
        int row;
        stream >> row;
        movedProviderIndexes.append(row);
    }

    for ( int i = 0 ; i < movedProviderIndexes.count(); i ++ )
    {
        int row = movedProviderIndexes.at(i);
        //if ( !beginMoveRows(parent, row, row, parent, beginRow) )
        //    continue;
        if ( beginRow >= maxRowCount )
            --beginRow;
        dataProviders->move(row, beginRow);
        //endMoveRows();
        emit rowMoved(row, beginRow);
    }

    return true;
}

//=========================================================================
Qt::DropActions DataSourcesModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


