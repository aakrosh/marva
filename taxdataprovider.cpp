#include "taxdataprovider.h"
#include "blast_data.h"
#include <QColor>
#include <QObject>

//=========================================================================
TaxDataProvider::TaxDataProvider(QObject *parent, TaxDataProviderType _type) : QObject(parent), type(_type)
{

}

//=========================================================================
quint32 TaxDataProvider::count()
{
    return idTaxNodeList.count();
}

//=========================================================================
qint32 TaxDataProvider::id(quint32 index)
{
    return idTaxNodeList.at(index).id;
}

//=========================================================================
BaseTaxNode *TaxDataProvider::taxNode(quint32 index)
{
    return idTaxNodeList.at(index).node;
}

//=========================================================================
quint32 TaxDataProvider::reads(quint32)
{
    return 0;
}

//=========================================================================
QString TaxDataProvider::text(quint32 index)
{
    return taxNode(index)->getText();
}

//=========================================================================
quint32 TaxDataProvider::readsById(quint32)
{
    return 0;
}

//=========================================================================
quint32 TaxDataProvider::indexOf(qint32 id)
{
    QList<IdTaxNodePair>::const_iterator it = idTaxNodeList.constBegin();
    while ( it != idTaxNodeList.end() )
    {
        if ( (*it).id == id )
            return it - idTaxNodeList.constBegin();
        ++it;
    }
    return -1;
}

//=========================================================================
QVariant TaxDataProvider::checkState(int index)
{
    if ( index < (int)count() )
        return idTaxNodeList.at(index).node->visible() ? Qt::Checked : Qt::Unchecked;
    else
        return TaxDataProvider::checkState(index);
}

//=========================================================================
void TaxDataProvider::setCheckedState(int index, QVariant value)
{
    bool visible = value == Qt::Checked;
    BaseTaxNode *node = idTaxNodeList.at(index).node;
    if ( visible != node->visible() )
        node->setVisible(visible);
}

//=========================================================================
bool nameLessThan(const IdTaxNodePair &v1, const IdTaxNodePair &v2)
{
    bool res = v1.node->getText() < v2.node->getText();
     return res ;
}
bool idLessThan(const IdTaxNodePair &v1, const IdTaxNodePair &v2)
{
     return v1.id < v2.id;
}

void TaxDataProvider::sort(int column, Qt::SortOrder order)
{
    if ( column == 2 )
        return;
    if ( column == 0 )
        qSort(idTaxNodeList.begin(), idTaxNodeList.end(), nameLessThan);
    else if ( column == 1 )
        qSort(idTaxNodeList.begin(), idTaxNodeList.end(), idLessThan);

    if ( order == Qt::DescendingOrder )
        std::reverse(idTaxNodeList.begin(), idTaxNodeList.end());
}

//=========================================================================
quint32 TaxDataProvider::getMaxReads() { return 0; }

//=========================================================================
void TaxDataProvider::onDataLoaded()
{
    updateCache(false);
    emit dataChanged();
}

//=========================================================================
GlobalTaxMapDataProvider::GlobalTaxMapDataProvider(QObject *parent, TaxMap *tm) :
    TaxDataProvider(parent, GLOBAL_DATA_PROVIDER),
    taxMap(tm)
{
    updateCache(false);
}

//=========================================================================
void GlobalTaxMapDataProvider::updateCache(bool values_only)
{
    if ( !values_only )
    {
        idTaxNodeList.clear();
        idTaxNodeList.reserve(taxMap->size());
    }
    int i = 0;
    for ( TaxMap::iterator mit = taxMap->begin(); mit != taxMap->end(); ++mit )
    {
        if ( values_only )
            idTaxNodeList[i++].node = mit.value();
        else
            idTaxNodeList.append(IdTaxNodePair(mit.value(), mit.key()));
    }
}

//=========================================================================
void GlobalTaxMapDataProvider::onTreeLoaded()
{
    onDataLoaded();
}

//=========================================================================
void GlobalTaxMapDataProvider::onMapChanged()
{
    updateCache(true);
    emit dataChanged();
}



//=========================================================================
BlastTaxDataProvider::BlastTaxDataProvider(QObject *parent):
    TaxDataProvider(parent, BLAST_DATA_PROVIDER)
{
    blastNodeMap = new BlastNodeMap();
}


//=========================================================================
quint32 BlastTaxDataProvider::reads(quint32 index)
{
    if ( idTaxNodeList.empty() )
        return 0;
    if ( (int)index >= idTaxNodeList.count() )
        return 0;
    BlastTaxNode *node = ((BlastTaxNode *)idTaxNodeList.at(index).node);
    if ( node == NULL )
        return 0;
    return node->reads;
}


//=========================================================================
quint32 BlastTaxDataProvider::readsById(quint32 id)
{
    if ( blastNodeMap->empty() )
        return 0;

    BlastTaxNode *node = blastNodeMap->value(id);
    if ( node == NULL )
        return 0;
    return node->reads;
}

//=========================================================================
void BlastTaxDataProvider::updateCache(bool values_only)
{
    QReadWriteLocker locker(&lock, true);

    if ( !values_only )
    {
        idTaxNodeList.clear();
        idTaxNodeList.reserve(blastNodeMap->size());
    }
    int i = 0;
    if ( idTaxNodeList.size() < blastNodeMap->size() )
        idTaxNodeList.reserve(blastNodeMap->size());
    for ( BlastNodeMap::iterator mit = blastNodeMap->begin(); mit != blastNodeMap->end(); ++mit )
    {
        if ( values_only && i < idTaxNodeList.size() )
        {
            idTaxNodeList[i++].node = mit.value();
        }
        else
        {
            idTaxNodeList.append(IdTaxNodePair(mit.value(), mit.key()));
        }
    }
}

//=========================================================================
QColor BlastTaxDataProvider::color(int index)
{
    QReadWriteLocker locker(&lock);
    if ( index < (int)count() )
        return QColor(taxColorSrc.getColor(idTaxNodeList.at(index).id)).lighter(150);
    else
        return TaxDataProvider::color(index);
}

//=========================================================================
bool readsLessThan(const IdTaxNodePair &v1, const IdTaxNodePair &v2)
{
     return ((BlastTaxNode *)v1.node)->reads < ((BlastTaxNode *)v2.node)->reads;
}

void BlastTaxDataProvider::sort(int column, Qt::SortOrder order)
{
    if ( column == 2 )
    {
        qSort(idTaxNodeList.begin(), idTaxNodeList.end(), readsLessThan);
        if ( order == Qt::DescendingOrder )
            std::reverse(idTaxNodeList.begin(), idTaxNodeList.end());
    }
    else
    {
        TaxDataProvider::sort(column, order);
    }
}

//=========================================================================
quint32 BlastTaxDataProvider::getMaxReads() { return blastNodeMap->max_reads; }

//=========================================================================
void BlastTaxDataProvider::onBlastProgress(void *)
{
    emit dataChanged();
}

//=========================================================================
void BlastTaxDataProvider::onBlastLoaded(void *)
{
    emit dataChanged();
}


