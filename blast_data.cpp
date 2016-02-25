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
    blastNodeMap.clear();
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
            BlastNodeMap::iterator bit = blastNodeMap.find(rec.taxa_id);
            if ( bit == blastNodeMap.end() )
            {
                BlastTaxNode *blastNode = new BlastTaxNode(node, 1);
                BlastTaxNode *res = blastNode->createPathToNode();
                if ( root == NULL )
                {
                    root = res;
                    result = res;
                }
                blastNodeMap.insert(node->getId(), blastNode);
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
BlastTaxNode::BlastTaxNode(TaxNode *refNode, int _count):BaseTaxNode(false), count(_count), tNode(refNode)
{
    blastNodeMap.insert(refNode->getId(), this);
}

//=========================================================================
BlastTaxNode *BlastTaxNode::createPathToNode()
{
    BlastTaxNode *curNode = this;
    while ( curNode->tNode->parent != NULL && curNode->parent == NULL )
    {
        BlastNodeMap::iterator it = blastNodeMap.find(curNode->tNode->parent->getId());
        BlastTaxNode *cur_parent =
                it == blastNodeMap.end()
              ? new BlastTaxNode((TaxNode *)curNode->tNode->parent, 0)
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
