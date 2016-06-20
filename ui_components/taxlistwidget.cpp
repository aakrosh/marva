#include "taxlistwidget.h"
#include "ui_taxlistwidget.h"
#include "tax_map.h"
#include "main_window.h"
#include "taxnodesignalsender.h"
#include "taxdataprovider.h"
#include "colors.h"

#include <QPaintEvent>
#include <QtConcurrent/QtConcurrent>
#include <functional>
#include <qDebug>
//=========================================================================
TaxListWidget::TaxListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaxListWidget),
    oldRowCount(0)
{
    ui->setupUi(this);
    model = new TaxListTableModel(this);
    ui->tableView->setModel(model);
    ui->tableView->installEventFilter(this);
    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(taxChanged(QModelIndex, QModelIndex)));
    connect(model, SIGNAL(layoutChanged()), ui->tableView->selectionModel(), SLOT(clear()));
    refresh();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(showContextMenu(const QPoint&)));
}

//=========================================================================
TaxListWidget::~TaxListWidget()
{
    delete ui;
}

//=========================================================================
void TaxListWidget::setTaxDataProvider(TaxDataProvider *tdp)
{
    if ( tdp == model->taxDataProvider )
        return;
    model->taxDataProvider = tdp;
    reset();
}

//=========================================================================
bool TaxListWidget::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->tableView && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space)
        {
            QModelIndexList sel = ui->tableView->selectionModel()->selectedRows(0);
            int sel_count =  sel.count();
            if ( sel_count == 0 )
                return false;
            bool checked = sel.at(0).data(Qt::CheckStateRole) != Qt::Checked;
            Qt::CheckState cs =  checked ? Qt::Checked : Qt::Unchecked;
            bool with_reset = sel_count > 100;
            if ( with_reset )
                getTaxNodeSignalSender(NULL)->sendSignals = false;
            for ( int i = 0 ; i < sel_count; i++ )
                model->setData(sel.at(i), cs, Qt::CheckStateRole);
            QModelIndex i1 = ui->tableView->indexAt(ui->tableView->rect().topLeft());
            QModelIndex i2 = ui->tableView->indexAt(ui->tableView->rect().bottomRight());
            model->dataChanged(i1, i2); // It is cheaper to refresh only visible items, then all the items in the selection
            if ( with_reset )
            {
                getTaxNodeSignalSender(NULL)->sendSignals = true;
                QtConcurrent::run(std::bind([] (QModelIndexList& ) {}, std::move(sel)));
                getTaxNodeSignalSender(NULL)->BigChangesHappened();
            }
            return true;
        }
    }
    return false;
}

//=========================================================================
void TaxListWidget::resetView()
{
    reset();
}

//=========================================================================
void TaxListWidget::reset()
{
    if ( model == NULL || model->taxDataProvider == NULL )
        return;
    oldRowCount = model->taxDataProvider->count();
    QModelIndexList sel = ui->tableView->selectionModel()->selectedRows(0);
    QList<int> rows;
    foreach(QModelIndex index, sel)
        rows.append(index.row());
    QtConcurrent::run(std::bind([] (QModelIndexList& ) {}, std::move(sel)));

    model->beginResetModel();
    model->clearCache();
    model->endResetModel();

    ui->tableView->setSelectionMode(QAbstractItemView::MultiSelection);
    if ( rows.count() == ui->tableView->model()->rowCount() )
        ui->tableView->selectAll();
    else
    {
        foreach(int row, rows)
            ui->tableView->selectRow(row);
    }
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

//=========================================================================
void TaxListWidget::refresh()
{
    if ( model == NULL || model->taxDataProvider == NULL )
        return;
    QModelIndex i1 = ui->tableView->indexAt(ui->tableView->rect().topLeft());
    QModelIndex i2 = ui->tableView->indexAt(ui->tableView->rect().bottomRight());
    quint32 old_count = model->taxDataProvider->count();
    model->taxDataProvider->updateCache(true);
    quint32 new_count = model->taxDataProvider->count();
    if ( old_count < new_count )
    {
        model->beginInsertRows(QModelIndex(), old_count, new_count-1);
        model->endInsertRows();
    }
    model->clearCache();
    model->dataChanged(i1, i2);

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
      if ( !index.isValid() )
        return;
      TaxNodeSignalSender *tnss = getTaxNodeSignalSender(model->taxDataProvider->taxNode(index.row()));
      tnss->makeCurrent();
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
void TaxListWidget::onColorChanged(BaseTaxNode *node)
{
    if ( model->taxDataProvider == NULL )
        return;
    qint32 index = model->taxDataProvider->indexOf(node->getId());
    if ( index < 0 )
        return;
    QModelIndex cell = model->index(index, 0);
    model->dataChanged(cell, cell);
}

//=========================================================================
void TaxListWidget::showContextMenu(const QPoint &p)
{
    QMenu popupMenu;
    QAction *colorsAction = popupMenu.addAction("Colors...");
    connect(colorsAction, SIGNAL(triggered(bool)), this, SLOT(changeCurrentTaxColor()));
    popupMenu.exec(mapToGlobal(p));

}

//=========================================================================
void TaxListWidget::changeCurrentTaxColor()
{
    QModelIndexList selRows = this->ui->tableView->selectionModel()->selectedRows();
    colors->pickColor(model->taxDataProvider->taxNode(selRows[0].row())->getId());
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
    QModelIndex cell = model->index(index, 0);
    model->dataChanged(cell, cell);
}

//=========================================================================
TaxListTableModel::TaxListTableModel(QObject *parent):
    QAbstractItemModel(parent),
    taxDataProvider(NULL)
{
}

//=========================================================================
TaxListTableModel::~TaxListTableModel()
{
    delete taxDataProvider;
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
    if ( !index.isValid() )
      return QVariant();

    if (role == Qt::DisplayRole)
    {
        qint32 i = index.row();
        if ( !cache.contains(i) )
            cache.addRecord(i, taxDataProvider->text(i), taxDataProvider->id(i), taxDataProvider->reads(i));
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
bool TaxListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( !index.isValid() )
      return false;

    if ( role == Qt::CheckStateRole && index.column() == 0 )
    {
        taxDataProvider->setCheckedState(index.row(), value);
        return true;
    }
    return false;
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
    emit layoutAboutToBeChanged();
    taxDataProvider->sort(column, order);
    //TaxNodeSignalSender *tnss = getTaxNodeSignalSender();
    clearCache();
    emit layoutChanged();
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
/*void TaxListTableModel::onLayoutChanged()
{

}*/

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
    if ( index.isValid() && index.column() == 0 )
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
