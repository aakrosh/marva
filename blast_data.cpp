#include "blast_data.h"
#include "main_window.h"
#include "graph_node.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <exception>
using namespace std;

BlastNodeMap blastNodeMap;
TaxColorSrc taxColorSrc;

//=========================================================================
BlastDataTreeLoader::BlastDataTreeLoader(QObject *parent, QString fileName, BlastFileType _type) :
    LoaderThread(parent, fileName, "Loading blast file", root),
    type(_type),
    root(NULL)
{
    blastNodeMap = new BlastNodeMap();
}

//=========================================================================
BlastDataTreeLoader::~BlastDataTreeLoader()
{
}

//=========================================================================
void BlastDataTreeLoader::processLine(QString &line)
{
    switch ( type )
    {
        case tabular:
        {
            QStringList list = line.split("\t", QString::SkipEmptyParts);
            BlastRecord rec(type, list);
            TaxMapIterator it = taxMap.find(rec.taxa_id);
            if ( it == taxMap.end() )
            {
                QMessageBox::information(0, QString("Error"), QString("No data for id %1 found").arg(rec.taxa_id));
                return;
            }
            TaxNode *node = it.value();
            BlastNodeMap::iterator bit = blastNodeMap->find(rec.taxa_id);
            if ( bit == blastNodeMap->end() )
            {
                BlastTaxNode *blastNode = new BlastTaxNode(node, 1, blastNodeMap);
                BlastTaxNode *res = blastNode->createPathToNode(blastNodeMap);
                //blastNodeMap->insert(node->getId(), blastNode);
                if ( root == NULL )
                {
                    root = res;
                    result = res;
                }
            }
            else
            {
                bit.value()->count++;
                GraphNode *gn = bit.value()->getGnode();
                if ( gn != NULL )
                    gn->markDirty(DIRTY_NAME);
            }
        }
        break;
        default:
        {
            throw("Unknow file format");
        }
    }
}

//=========================================================================
BlastTaxNode::BlastTaxNode(TaxNode *refNode, int _count, BlastNodeMap *blastNodeMap):BaseTaxNode(false), count(_count), tNode(refNode)
{
    blastNodeMap->insert(refNode->getId(), this);
    visible = true;
}

//=========================================================================
BlastTaxNode *BlastTaxNode::createPathToNode(BlastNodeMap *blastNodeMap)
{
    BlastTaxNode *curNode = this;
    while ( curNode->tNode->parent != NULL && curNode->parent == NULL )
    {
        BlastNodeMap::iterator it = blastNodeMap->find(curNode->tNode->parent->getId());
        BlastTaxNode *cur_parent =
                it == blastNodeMap->end()
              ? new BlastTaxNode((TaxNode *)curNode->tNode->parent, 0, blastNodeMap)
              : it.value();
        cur_parent->addChild(curNode);
        curNode = cur_parent;
    }
    return curNode;
}

#include "graph_node.h"
//=========================================================================
GraphNode *BlastTaxNode::createGnode(GraphView *gv)
{
    gnode = new BlastGraphNode(gv, this);
    return gnode;
}

//=========================================================================
quint32 TaxColorSrc::getColor(qint32 tax_id)
{
    iterator it = find(tax_id);
    if ( it == end() )
    {
        quint32 color = ((rand() % 0xFFFF) + ((rand() % 0xFF) << 16)) | 0xFF000000;
        insert(tax_id, color);
        return color;
    }
    else
    {
        return it.value();
    }
}

//=========================================================================
BlastTaxDataProvider::BlastTaxDataProvider(BlastNodeMap *bnm):TaxDataProvider(), blastNodeMap(bnm){}

//=========================================================================
quint32 BlastTaxDataProvider::count()
{
    QReadWriteLocker locker(&lock);
    return ids.count();
}

//=========================================================================
qint32 BlastTaxDataProvider::id(quint32 index)
{
    QReadWriteLocker locker(&lock);
    return ids.at(index);
}

//=========================================================================
BaseTaxNode *BlastTaxDataProvider::taxNode(quint32 index)
{
    QReadWriteLocker locker(&lock);
    if ( index >= (quint32)taxnodes.count() )
        return NULL;
    return (BaseTaxNode *)taxnodes.at(index);
}

//=========================================================================
quint32 BlastTaxDataProvider::reads(quint32 index)
{
    return taxnodes.at(index)->count;
}

//=========================================================================
quint32 BlastTaxDataProvider::indexOf(qint32 id)
{
    QReadWriteLocker locker(&lock);
    return ids.indexOf(id);
}

//=========================================================================
void BlastTaxDataProvider::updateCache(bool values_only)
{
    QReadWriteLocker locker(&lock, true);
    taxnodes = blastNodeMap->values();
    if ( !values_only )
        ids = blastNodeMap->keys();
}

//=========================================================================
QColor BlastTaxDataProvider::color(int index)
{
    QReadWriteLocker locker(&lock);
    if ( index < (int)count() )
        return QColor(taxColorSrc.getColor(ids.at(index))).lighter(150);
    else
        return TaxDataProvider::color(index);
}

//=========================================================================
QVariant BlastTaxDataProvider::checkState(int index)
{
    QReadWriteLocker locker(&lock);
    if ( index < (int)count() )
        return taxnodes.at(index)->visible ? Qt::Checked : Qt::Unchecked;
    else
        return TaxDataProvider::checkState(index);
}

//=========================================================================
void BlastTaxDataProvider::setCheckedState(int index, QVariant value)
{
    bool visible = value == Qt::Checked;
    QReadWriteLocker locker(&lock);
    if ( visible != taxnodes.at(index)->visible )
        taxnodes.at(index)->visible = value == Qt::Checked;
}
