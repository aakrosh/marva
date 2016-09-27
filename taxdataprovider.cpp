#include "taxdataprovider.h"
#include "blast_data.h"
#include "graph_node.h"
#include "colors.h"
#include "logging.h"
#include "main_window.h"

#include <QColor>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

BlastTaxDataProviders blastTaxDataProviders;
GlobalTaxMapDataProvider *globalTaxDataProvider;
//=========================================================================
TaxDataProvider::TaxDataProvider(QObject *parent, TaxDataProviderType _type)
    : QObject(parent),
      current_tax_id(0),
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
BaseTaxNode *TaxDataProvider::taxNode(qint32 index)
{
    if ( ((qint32)index) < 0 )
        return NULL;
    QReadWriteLocker locker(&lock);

    return idTaxNodeList.at(index).node;
}

//=========================================================================
BaseTaxNode *TaxDataProvider::taxNodeById(qint32 id)
{
    return taxNode(indexOf(id));
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
quint32 TaxDataProvider::sumById(quint32)
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
qint32 TaxDataProvider::indexOf(qint32 id)
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
    root(NULL),
    totalReads(0)
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
        delete this;
}

//=========================================================================
BlastTaxNode *BlastTaxDataProvider::addTaxNode(qint32 id, qint32 reads, quint64 pos)
{
    QVector<quint64> positions;
    positions.append(pos);
    return addTaxNode(id, reads, positions);
}

//=========================================================================
BlastTaxNode *BlastTaxDataProvider::addTaxNode(qint32 id, qint32 reads, QVector<quint64> &pos)
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
        blastNode = bit.value();
    }
    blastNode->positions.append(pos);
    if ( (qint32)blastNodeMap->max_reads < reads )
        blastNodeMap->max_reads = reads;
    totalReads++;
    return blastNode;
}

//=========================================================================
void BlastTaxDataProvider::toJson(QJsonObject &json)
{
    mlog.log(QString("Start saving data provider %1 to json").arg(name));
    json["Name"] = name;
    json["Filename"] = fileName;
    QJsonArray dpArray;
    /*
    for (int i = 0 ; i < idTaxNodeList.count(); i++ )
    {
        const IdTaxNodePair &pair = idTaxNodeList[i];
        BlastTaxNode *node = ((BlastTaxNode *)pair.node);
        if ( node->reads == 0 && node->visible() && !node->isCollapsed() )
            continue;           // This nodes will be generated during the loading automatically
        QJsonArray jnode;
        jnode.append(pair.id);
        jnode.append((qint64)node->reads);
        jnode.append(node->visible() ? 1 : 0);
        jnode.append(node->isCollapsed() ? 1: 0);
        QJsonArray jpos;
        for ( int j = 0; j < node->positions.count(); j++)
            jpos.append((qint64)node->positions[j]);
        jnode.append(jpos);
        dpArray.append(jnode);
    }
    json["Arr"] = dpArray;
*/
    QStringList snodes;
    QReadWriteLocker locker(&lock);
    QJsonArray jArrays;
    for ( int i=0; i < idTaxNodeList.count(); i++ )
    {
        const IdTaxNodePair &pair = idTaxNodeList[i];
        BlastTaxNode *node = ((BlastTaxNode *)pair.node);
        if ( node->reads == 0 && node->visible() && !node->isCollapsed() )
            continue;           // This nodes will be generated during the loading automatically
        QStringList sposes;
        for ( int j = 0; j < node->positions.count(); j++)
            sposes.append(QString::number((qint64)node->positions[j]));
        QString snode = QString("[%1,%2,%3,%4,[%5]]")
                .arg(pair.id)
                .arg((quint64)node->reads)
                .arg(node->visible() ? 1 : 0)
                .arg(node->isCollapsed() ? 1 : 0)
                .arg(sposes.join(','));
        snodes.append(snode);
        if ( snodes.count() == 100 )
        {
            QJsonDocument jd = QJsonDocument::fromJson(QString("[%1]").arg(snodes.join(',')).toUtf8());
            QJsonArray jArr = jd.array();
            jArrays.append(jArr);
            snodes.clear();
        }
    }
    if ( snodes.count() > 0 )
    {
        QJsonDocument jd = QJsonDocument::fromJson(QString("[%1]").arg(snodes.join(',')).toUtf8());
        QJsonArray jArr = jd.array();
        jArrays.append(jArr);
    }
    json["Arr"] = jArrays;
    mlog.log(QString("End saving data provider %1 to json").arg(name));
}

//=========================================================================
void BlastTaxDataProvider::fromJson(QJsonObject &json)
{
    try
    {
        name = json["Name"].toString();
        fileName = json["Filename"].toString();
        QJsonArray dpArr = json["Arr"].toArray();
        for (int i = 0; i < dpArr.size(); ++i)
        {
            QJsonArray jNode = dpArr[i].toArray();
            qint32 id = jNode[0].toInt();
            quint32 reads = jNode[1].toInt();
            bool visible = jNode[2].toInt() == 1;
            bool collapsed = jNode[3].toInt() == 1;
            QJsonArray jPos = jNode[4].toArray();
            QVector<quint64> positions;
            for ( int j = 0; j < jPos.count(); j++ )
                positions.append(jPos[j].toInt());
            BlastTaxNode *node = addTaxNode(id, reads > 0 ? reads : -1, positions);
            if ( node != NULL )
            {
                node->setVisible(visible);
                if ( collapsed )
                    node->setCollapsed(true, false);
            }
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

static void writeByteArrayToFile(const QByteArray &ba, QFile &file)
{
    quint32 size = ba.size();
    file.write((const char *)&size, sizeof(size));
    file.write(ba);
}

static void readByteArrayFromFile(QByteArray &ba, QFile &file)
{
    quint32 size;
    file.read((char *)&size, sizeof(size));
    ba = file.read(size);
}

static void writeJsonObjectToFile(QJsonObject &jobj, QFile &file)
{
    QJsonDocument jdoc(jobj);
    writeByteArrayToFile(jdoc.toBinaryData(), file);
}

static void readJsonObjectFromFile(QJsonObject &jobj, QFile &file)
{
    QByteArray ba;
    readByteArrayFromFile(ba, file);
    QJsonDocument jdoc = QJsonDocument::fromBinaryData(ba);
    jobj = jdoc.object();
}

static void writeJsonArrayToFile(QJsonArray &jarr, QFile &file)
{
    QJsonDocument jdoc(jarr);
    writeByteArrayToFile(jdoc.toBinaryData(), file);
}

static void readJsonArrayFromFile(QJsonArray &jarr, QFile &file)
{
    QByteArray ba;
    readByteArrayFromFile(ba, file);
    QJsonDocument jdoc = QJsonDocument::fromBinaryData(ba);
    jarr = jdoc.array();
}

void BlastTaxDataProvider::serialize(QFile &file)
{
    mlog.log(QString("Start saving data provider %1 to json").arg(name));
    QJsonObject json;
    json["Name"] = name;
    json["Filename"] = fileName;
    writeJsonObjectToFile(json, file);

    quint32 size = idTaxNodeList.count();
    file.write((const char *)&size, sizeof(size));
    for (int i = 0 ; i < idTaxNodeList.count(); i++ )
    {
        const IdTaxNodePair &pair = idTaxNodeList[i];
        BlastTaxNode *node = ((BlastTaxNode *)pair.node);
        QJsonArray jnode;
        jnode.append(pair.id);
        jnode.append((qint64)node->reads);
        jnode.append(node->visible() ? 1 : 0);
        jnode.append(node->isCollapsed() ? 1: 0);
        writeJsonArrayToFile(jnode, file);

        quint64 *data = node->positions.data();
        size = node->positions.size() * sizeof(quint64);
        file.write((const char *)&size, sizeof(size));
        file.write((const char *)data, size);
    }
    mlog.log(QString("Finished saving data provider %1 to json").arg(name));
}

//=========================================================================
void BlastTaxDataProvider::deserialize(QFile &file, qint32 /*version*/)
{
    QJsonObject json;
    readJsonObjectFromFile(json, file);
    name = json["Name"].toString();
    fileName = json["Filename"].toString();
    totalReads = 0;

    quint32 size;
    file.read((char *)&size, sizeof(size));
    for (quint32  i = 0; i < size; ++i)
    {
        QJsonArray jNode;
        readJsonArrayFromFile(jNode, file);
        qint32 id = jNode[0].toInt();
        quint32 reads = jNode[1].toInt();
        bool visible = jNode[2].toInt() == 1;
        bool collapsed = jNode[3].toInt() == 1;

        totalReads += reads;

        quint32 psize;
        file.read((char *)&psize, sizeof(psize));


        QByteArray ba = file.read(psize);
        quint64 *arr = (quint64 *)ba.data();
        std::vector<quint64> tmp;
        tmp.assign(arr, arr + psize/sizeof(quint64));
        QVector<quint64> positions = QVector<quint64>::fromStdVector(tmp);

        BlastTaxNode *node = addTaxNode(id, reads > 0 ? reads : -1, positions);
        if ( node != NULL )
        {
            node->setVisible(visible);
            if ( collapsed )
                node->setCollapsed(true, false);
        }
    }
    updateCache(false);
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
quint32 BlastTaxDataProvider::sumById(quint32 id)
{
    if ( blastNodeMap->empty() )
        return 0;

    BlastTaxNode *node = blastNodeMap->value(id);
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
bool sumLessThan(const IdTaxNodePair &v1, const IdTaxNodePair &v2)
{
     return ((BlastTaxNode *)v1.node)->reads < ((BlastTaxNode *)v2.node)->reads;
}

void BlastTaxDataProvider::sort(int column, Qt::SortOrder order)
{
    if ( column == 2 )
    {
        QReadWriteLocker locker(&lock, true);
        qSort(idTaxNodeList.begin(), idTaxNodeList.end(), sumLessThan);
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
    visibility_mask(0),
    serializingProviders(0)
{
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

//=========================================================================
void BlastTaxDataProviders::addProvider(BlastTaxDataProvider *p)
{
    append(p);
    setVisible(count()-1, true);
}

//=========================================================================
ProvidersSerializationThread::ProvidersSerializationThread(BlastTaxDataProvider *p):
    QThread(),
    provider(p)
{

}

//=========================================================================
void ProvidersSerializationThread::run()
{
    provider->toJson(json);
    emit completed(this);
}
