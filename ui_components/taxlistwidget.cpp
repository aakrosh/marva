#include "taxlistwidget.h"
#include "ui_taxlistwidget.h"
#include "tax_map.h"
#include "main_window.h"

#include <QPaintEvent>

//=========================================================================
TaxListWidget::TaxListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaxListWidget)
{
    ui->setupUi(this);
    model = new TaxListTableModel(this);
    ui->tableView->setModel(model);
    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(taxChanged(QModelIndex, QModelIndex)));
}

//=========================================================================
TaxListWidget::~TaxListWidget()
{
    delete ui;
}

//=========================================================================
void TaxListWidget::reset()
{
    ui->tableView->setModel(NULL);
    ui->tableView->setModel(model);
}

//=========================================================================
void TaxListWidget::refresh()
{
    if ( taxMap.empty() )
        return;
    model->setTaxMap(&taxMap, false);
    static int oldRowCount = 0;
    if ( oldRowCount >= taxMap.count() )
        return;
    model->beginInsertRows(QModelIndex(), oldRowCount, taxMap.count()-1);
    model->endInsertRows();
    oldRowCount = taxMap.count();
    refreshValues();
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
    emit currentTaxChanged(model->taxnodes.at(index.row()));
}

//=========================================================================
void TaxListWidget::onCurrentTaxChanged(BaseTaxNode *node)
{
    quint32 index = model->ids.indexOf(node->getId());
    if ( index < 0 )
        return;
    if ( index == ui->tableView->currentIndex().row() )
        return;
    ui->tableView->selectRow(index);
    ui->tableView->scrollTo(model->index(index, 0));
}

//=========================================================================
TaxListTableModel::TaxListTableModel(QObject *parent):
    QAbstractItemModel(parent),
    listTaxMap(&taxMap)
{
    taxnodes = listTaxMap->values();
    ids = listTaxMap->keys();
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
    if ( listTaxMap == NULL )
        return 0;
    return listTaxMap->count();
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
            cache.addRecord(i, taxnodes[i]->getText(), ids[i], 0);
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

    return QVariant();
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
void TaxListTableModel::sort(int column, Qt::SortOrder order)
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
void TaxListTableModel::setTaxMap(TaxMap *taxMap, bool refreshValuesOnly=false)
{
    if ( !refreshValuesOnly )
    {
        listTaxMap = taxMap;
        ids = taxMap->keys();
    }
    taxnodes = taxMap->values();
    clearCache();
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
