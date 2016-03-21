#include "taxlistwidget.h"
#include "ui_taxlistwidget.h"
#include "tax_map.h"
#include "main_window.h"
#include "taxnodesignalsender.h"

#include <QPaintEvent>

//=========================================================================
TaxListWidget::TaxListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaxListWidget),
    oldRowCount(0)
{
    ui->setupUi(this);
    model = new TaxListTableModel(this);
    ui->tableView->setModel(model);
    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(taxChanged(QModelIndex, QModelIndex)));
    refresh();
}

//=========================================================================
TaxListWidget::~TaxListWidget()
{
    delete ui;
}

//=========================================================================
void TaxListWidget::reset()
{
    oldRowCount = model->taxDataProvider->count();
    model->beginResetModel();
    model->endResetModel();
}

//=========================================================================
void TaxListWidget::refresh()
{
    if ( model->taxDataProvider == NULL )
        model->setTaxDataProvider(new GlobalTaxMapDataProvider(), false);
    else
        model->setTaxDataProvider(NULL, false);

    reset();
    return;
}

//=========================================================================
void TaxListWidget::refreshAll()
{
    model->taxDataProvider->updateCache(false);
    reset();
}

//=========================================================================
void TaxListWidget::refreshValues()
{
    model->clearCache();
    model->emitDataChangedSignal(ui->tableView->indexAt(ui->tableView->rect().topLeft()), ui->tableView->indexAt(ui->tableView->rect().bottomRight()));
}

//=========================================================================
void TaxListWidget::taxChanged(QModelIndex index, QModelIndex)
{
    emit currentTaxChanged(model->taxDataProvider->taxNode(index.row()));
}

//=========================================================================
void TaxListWidget::onCurrentTaxChanged(BaseTaxNode *node)
{
    if ( model->taxDataProvider == NULL )
        return;
    qint32 index = model->taxDataProvider->indexOf(node->getId());
    if ( index < 0 )
        return;
    if ( index == ui->tableView->currentIndex().row() )
        return;
    ui->tableView->selectRow(index);
    ui->tableView->scrollTo(model->index(index, 0));
}

//=========================================================================
void TaxListWidget::onNodeVisibilityChanged(BaseTaxNode *node, bool visible)
{
    if ( model->taxDataProvider == NULL )
        return;
    qint32 index = model->taxDataProvider->indexOf(node->getId());
    if ( index < 0 )
        return;
    model->taxDataProvider->setCheckedState(index, visible ? Qt::Checked : Qt::Unchecked);
}

//=========================================================================
TaxListTableModel::TaxListTableModel(QObject *parent):
    QAbstractItemModel(parent),
    taxDataProvider(NULL)
//    listTaxMap(&taxMap)
{
    //taxnodes = listTaxMap->values();
    //ids = listTaxMap->keys();
}

//=========================================================================
QModelIndex TaxListTableModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

//=========================================================================
QModelIndex TaxListTableModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

//=========================================================================
int TaxListTableModel::rowCount(const QModelIndex &) const
{
    if ( taxDataProvider == NULL )
        return 0;
    return taxDataProvider->count();
}

//=========================================================================
int TaxListTableModel::columnCount(const QModelIndex &) const
{
    return 3;
}

TaxListCache cache;
//=========================================================================
QVariant TaxListTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        qint32 i = index.row();
        if ( !cache.contains(i) )
        {
//            cache.addRecord(i, taxnodes[i]->getText(), ids[i], 0);
            BaseTaxNode *n = taxDataProvider->taxNode(i);
            if ( n == NULL )
                return QVariant();
            cache.addRecord(i, n->getText(), taxDataProvider->id(i), taxDataProvider->reads(i));
        }
        TaxListCacheData *data = cache.value(i);
        switch( index.column() )
        {
            case 0:
                return data->text;
            case 1:
                return data->id;
            case 2:
                return data->reads;
        }

    }
    else if (role == Qt::BackgroundColorRole)
    {
        return QVariant(taxDataProvider->color(index.row()));
    }
    else if (role == Qt::CheckStateRole && index.column() == 0 )
    {
        return QVariant(taxDataProvider->checkState(index.row()));
    }
    return QVariant();
}

//=========================================================================
bool TaxListTableModel::setData(const QModelIndex & index, const QVariant &value, int role)
{
    if ( role == Qt::CheckStateRole && index.column() == 0 )
    {
        taxDataProvider->setCheckedState(index.row(), value);
        getTaxNodeSignalSender(taxDataProvider->taxNode(index.row()))->VisibilityChanged(value==Qt::Checked);
    }
    return true;
}
//=========================================================================
QVariant TaxListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( orientation == Qt::Horizontal )
        {
            QStringList lst;
            lst << "Taxonomy" << "id" << "reads";
            if ( section < lst.size() )
                return lst[section];
        }
    }
    return QVariant();
}

//=========================================================================
void TaxListTableModel::sort(int /*column*/, Qt::SortOrder /*order*/)
{
    // view->sort(column, order);
}

//=========================================================================
void TaxListTableModel::emitDataChangedSignal(const QModelIndex &start, const QModelIndex &end)
{
    emit dataChanged(start, end);
}

//=========================================================================
void TaxListTableModel::clearCache()
{
    cache.clear();
}

//=========================================================================
void TaxListTableModel::setTaxDataProvider(TaxDataProvider *tdp, bool refreshValuesOnly=false)
{
    if ( tdp != NULL)
        taxDataProvider = tdp;
    taxDataProvider->updateCache(refreshValuesOnly);
    clearCache();
}

//=========================================================================
Qt::ItemFlags TaxListTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f =  Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if ( index.column() == 0 )
        f |= Qt::ItemIsUserCheckable;
    return f;
}

//=========================================================================
TaxListCache::TaxListCache(int msize): max_size(msize){}

//=========================================================================
void TaxListCache::addRecord(int i, QString text, qint32 id, quint32 reads)
{
    if ( size() == max_size )
    {
        int oldId = ids.at(0);
        ids.removeFirst();
        TaxListCacheData *container = value(oldId);
        remove(oldId);
        container->text = text;
        container->id = id;
        container->reads = reads;
        insert(i, container);
        ids.append(i);
    }
    else
    {
        insert(i, new TaxListCacheData(text, id, reads));
        ids.append(i);
    }
}

//=========================================================================
void TaxListCache::clear()
{
    QMap<int, TaxListCacheData *>::clear();
    ids.clear();
}

//=========================================================================
GlobalTaxMapDataProvider::GlobalTaxMapDataProvider()
{
    updateCache(false);
}

//=========================================================================
quint32 GlobalTaxMapDataProvider::count()
{
    return ids.count();
}

//=========================================================================
qint32 GlobalTaxMapDataProvider::id(quint32 index)
{
    return ids.at(index);
}

//=========================================================================
BaseTaxNode *GlobalTaxMapDataProvider::taxNode(quint32 index)
{
    return (BaseTaxNode *)taxnodes.at(index);
}

//=========================================================================
quint32 GlobalTaxMapDataProvider::reads(quint32)
{
    return 0;
}

//=========================================================================
quint32 GlobalTaxMapDataProvider::indexOf(qint32 id)
{
    return ids.indexOf(id);
}

//=========================================================================
void GlobalTaxMapDataProvider::updateCache(bool values_only)
{
    taxnodes = taxMap.values();
    if ( !values_only )
        ids = taxMap.keys();
}

//=========================================================================
QVariant GlobalTaxMapDataProvider::checkState(int index)
{
    if ( index < (int)count() )
        return taxnodes.at(index)->visible ? Qt::Checked : Qt::Unchecked;
    else
        return TaxDataProvider::checkState(index);
}

//=========================================================================
void GlobalTaxMapDataProvider::setCheckedState(int index, QVariant value)
{
    bool visible = value == Qt::Checked;
    if ( visible != taxnodes.at(index)->visible )
        taxnodes.at(index)->visible = visible;
}
