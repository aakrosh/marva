#include "tax_map.h"
#include "graph_node.h"
#include "graphview.h"
#include "edge.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

//=========================================================================
TaxMap::TaxMap() : QMap<qint32, TaxNode *>(){}

//=========================================================================
void TaxMap::setName(qint32 tid, const char *name, TaxRank rank)
{
    TaxMapIterator it = find(tid);
    if ( it == end() )
    {
        QMessageBox::information(0, "Cannot set name", QString("No node with id %1 found").arg(tid));
        return;
    }
    it.value()->text = name;
    if ( rank != TR_NORANK )
        it.value()->rank = rank;
}

//=========================================================================
TaxNode::TaxNode(): TreeTaxNode(true), id(-555), level(0){}

//=========================================================================
TaxNode::TaxNode(qint32 _id, TaxRank _rank): TreeTaxNode(true), id(_id), level(0), rank(_rank) {}

//=========================================================================
TaxNode *TaxNode::TaxNode::addChildById(quint32 chId, TaxRank rank)
{
    return (TaxNode *)addChild(new TaxNode(chId, rank));
}

//=========================================================================
GraphNode *TaxNode::createGnode(TreeGraphView *gv)
{
    gnode = new TaxTreeGraphNode(gv, this);
    return gnode;
}
