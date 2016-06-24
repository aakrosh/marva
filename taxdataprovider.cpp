#include "taxdataprovider.h"
#include "blast_data.h"
#include "graph_node.h"
#include "colors.h"

#include <QColor>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

BlastTaxDataProviders blastTaxDataProviders;
GlobalTaxMapDataProvider *globalTaxDataProvider;
//=========================================================================
TaxDataProvider::TaxDataProvider(QObject *parent, TaxDataProviderType _type)
    : QObject(parent),
      type(_type)
{

}

//=========================================================================
TaxDataProvider::~TaxDataProvider()
{

}

//=========================================================================
quint32 TaxDataProvider::count()
{
    QReadWriteLocker locker(&lock);

    return idTaxNodeList.count();
}

//=========================================================================
qint32 TaxDataProvider::id(quint32 index)
{
    QReadWriteLocker locker(&lock);

    return idTaxNodeList.at(index).id;
}

//=========================================================================
BaseTaxNode *TaxDataProvider::taxNode(quint32 index)
{
    QReadWriteLocker locker(&lock);

    return idTaxNodeList.at(index).node;
}

//=========================================================================
quint32 TaxDataProvider::reads(quint32)
{
    return 0;
}

//=========================================================================
quint32 TaxDataProvider::sum(quint32)
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
    QReadWriteLocker locker(&lock);

    if ( idTaxNodeList.empty() )
        return -1;
    IdTaxNodePair temp(NULL, id);
    return  idTaxNodeList.indexOf(temp);
}

//=========================================================================
QVariant TaxDataProvider::checkState(int index)
{
    QReadWriteLocker locker(&lock);

    if ( index < (int)count() )
        return idTaxNodeList.at(index).node->visible() ? Qt::Checked : Qt::Unchecked;
    else
        return TaxDataProvider::checkState(index);
}

//=========================================================================
void TaxDataProvider::setCheckedState(int index, QVariant value)
{
    QReadWriteLocker locker(&lock);

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
    QReadWriteLocker locker(&lock, true);

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
    QReadWriteLocker locker(&lock, true);

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
    TaxDataProvider(NULL, BLAST_DATA_PROVIDER),
    parent_count(0),
    root(NULL)
{
    blastTaxDataProviders.addProvider(this);
    blastNodeMap = new BlastNodeMap();
    if ( parent != NULL )
        addParent();
}

//=========================================================================
BlastTaxDataProvider::~BlastTaxDataProvider()
{
    blastTaxDataProviders.removeOne(this);
}

//=========================================================================
void BlastTaxDataProvider::addParent()
{
    ++parent_count;
}

void BlastTaxDataProvider::removeParent()
{
    if ( --parent_count == 0 )
        deleteLater();
}

//=========================================================================
BlastTaxNode *BlastTaxDataProvider::addTaxNode(qint32 id, qint32 reads, qint64 pos)
{
    QVector<qint64> positions;
    positions.append(pos);
    return addTaxNode(id, reads, positions);
}

//=========================================================================
BlastTaxNode *BlastTaxDataProvider::addTaxNode(qint32 id, qint32 reads, QVector<qint64> pos)
{
    TaxMapIterator it = taxMap.find(id);
    if ( it == taxMap.end() )
        return NULL;
    TreeTaxNode *node = it.value();
    BlastNodeMap::iterator bit = blastNodeMap->find(id);
    BlastTaxNode *blastNode = NULL;
    if ( bit == blastNodeMap->end() )
    {
        blastNode = new BlastTaxNode(node, 1, blastNodeMap);
        BlastTaxNode *res = blastNode->createPathToNode(blastNodeMap);
        if ( root == NULL )
            root = res;
        if ( reads > 0 )
            blastNode->reads = reads;
        blastNode->positions.append(pos);
    }
    else
    {
        if ( reads < 0 )
            reads = ++bit.value()->reads;
        else
            bit.value()->reads = reads;
        TaxTreeGraphNode *gn = (TaxTreeGraphNode *)bit.value()->getGnode();
        if ( gn != NULL )
            gn->markDirty(DIRTY_NAME);
    }
    if ( (qint32)blastNodeMap->max_reads < reads )
        blastNodeMap->max_reads = reads;
    return blastNode;
}

//=========================================================================
void BlastTaxDataProvider::toJson(QJsonObject &json)
{
    json["Name"] = name;
    QJsonArray dpArray;
    QReadWriteLocker locker(&lock);

    for (int i = 0 ; i < idTaxNodeList.count(); i++ )
    {
        const IdTaxNodePair &pair = idTaxNodeList[i];
        BlastTaxNode *node = ((BlastTaxNode *)pair.node);
        if ( node->reads == 0 && node->visible() && !node->isCollapsed() )
            continue;           // This nodes will be generated during the loading automatically
        QJsonArray jnode;
        jnode.append(pair.id);
        jnode.append((qint64)node->reads);
        jnode.append(node->is_visible ? 1 : 0);
        jnode.append(node->isCollapsed() ? 1: 0);
        QJsonArray jpos;
        for ( int j = 0; j < node->positions.count(); j++)
            jpos.append(node->positions[j]);
        jnode.append(jpos);
        dpArray.append(jnode);
    }
    json["Arr"] = dpArray;
}

//=========================================================================
void BlastTaxDataProvider::fromJson(QJsonObject &json)
{
    try
    {
        name = json["Name"].toString();
        QJsonArray dpArr = json["Arr"].toArray();
        for (int i = 0; i < dpArr.size(); ++i)
        {
            QJsonArray jNode = dpArr[i].toArray();
            qint32 id = jNode[0].toInt();
            quint32 reads = jNode[1].toInt();
            bool visible = jNode[2].toInt() == 1;
            bool collapsed = jNode[3].toInt() == 1;
            BlastTaxNode *node = addTaxNode(id, reads > 0 ? reads : -1);
            if ( node != NULL )
            {
                node->setVisible(visible);
                if ( collapsed )
                    node->setCollapsed(true, false);
            }
            QJsonArray jPos = jNode[4].toArray();
            for ( int j = 0; j < jPos.count(); j++ )
                node->positions.append(jPos[j].toInt());
        }
        updateCache(false);
    }
    catch (...)
    {
        QMessageBox::warning(NULL, "Error occured", "Cannot restore data for tree view");
    }
}

//=========================================================================
BlastTaxNode *BlastTaxDataProvider::nodeById(qint32 id)
{
    return blastNodeMap->value(id);
}

//=========================================================================
quint32 BlastTaxDataProvider::reads(quint32 index)
{
    QReadWriteLocker locker(&lock);

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
quint32 BlastTaxDataProvider::sum(quint32 index)
{
    QReadWriteLocker locker(&lock);

    if ( idTaxNodeList.empty() )
        return 0;
    if ( (int)index >= idTaxNodeList.count() )
        return 0;
    BlastTaxNode *node = ((BlastTaxNode *)idTaxNodeList.at(index).node);
    if ( node == NULL )
        return 0;
    return node->sum();
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
    if ( idTaxNodeList.size() < blastNodeMap->size() )
    {
        idTaxNodeList.reserve(blastNodeMap->size());
        values_only = false;
    }

    if ( values_only )
    {
        for ( int i = 0; i < idTaxNodeList.count(); i++ )
            if ( idTaxNodeList[i].node == NULL || idTaxNodeList[i].node->getText().isEmpty() )
                idTaxNodeList[i].node = blastNodeMap->value(idTaxNodeList[i].id);
    }
    else
    {
        for ( BlastNodeMap::iterator mit = blastNodeMap->begin(); mit != blastNodeMap->end(); ++mit )
            idTaxNodeList.append(IdTaxNodePair(mit.value(), mit.key()));
    }
}

//=========================================================================
QColor BlastTaxDataProvider::color(int index)
{
    QReadWriteLocker locker(&lock);
    if ( index < (int)count() )
        return QColor(colors->getColor(idTaxNodeList.at(index).id)).lighter(150);
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
        QReadWriteLocker locker(&lock, true);
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

//=========================================================================
BlastTaxDataProviders::BlastTaxDataProviders() :
    QList<BlastTaxDataProvider *>(),
    visibility_mask(0)
{
}

//=========================================================================
void BlastTaxDataProviders::toJson(QJsonObject &json)
{
    QJsonArray jProviders;
    for ( int  i = 0; i < count(); i++ )
    {
        QJsonObject jProvider;
        at(i)->toJson(jProvider);
        jProviders.append(jProvider);
    }
    json["BlastTaxDataProviders"] = jProviders;
}

//=========================================================================
void BlastTaxDataProviders::fromJson(QJsonObject &json)
{
    try
    {
        QJsonArray jProviders = json["BlastTaxDataProviders"].toArray();
        for ( int i = 0 ; i < jProviders.count(); i++ )
        {
            BlastTaxDataProvider *provider = new BlastTaxDataProvider(NULL);
            QJsonObject jProvider = jProviders[i].toObject();
            provider->fromJson(jProvider);
        }
    }
    catch (...)
    {
        QMessageBox::warning(NULL, "Error occured", "Cannot restore data for tree views");
    }
}

//=========================================================================
BlastTaxDataProvider *BlastTaxDataProviders::providerByName(const QString &name) const
{
    for ( int i = 0; i < count(); i++ )
    {
        BlastTaxDataProvider *p = at(i);
        if ( p->name == name )
            return p;
    }
    QMessageBox::warning(0, "Cannot find provider", QString("Cannot find provider with name %1").arg(name));
    return NULL;
}

//=========================================================================
void BlastTaxDataProviders::setVisible(quint8 index, bool visible)
{
    if ( visible )
        visibility_mask |= 1 << index;
    else
        visibility_mask &= ~(1 << index);
}

//=========================================================================
bool BlastTaxDataProviders::isVisible(quint8 index)
{
    return ((visibility_mask >> index) & 1) != 0;
}

void BlastTaxDataProviders::addProvider(BlastTaxDataProvider *p)
{
    append(p);
    setVisible(count()-1, true);
}
