#include "blast_data.h"
#include "main_window.h"
#include "graph_node.h"
#include "taxdataprovider.h"

#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <exception>
using namespace std;

BlastNodeMap blastNodeMap;
TaxColorSrc taxColorSrc;

//=========================================================================
BlastDataTreeLoader::BlastDataTreeLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp, BlastFileType _type) :
    LoaderThread(parent, fileName, NULL, NULL),
    type(_type),
    root(NULL),
    dataProvider(dp)
{
    dp->name = QFileInfo(fileName).fileName();
    caption = QString("Importing blast file: ").append(dp->name);
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
            dataProvider->addTaxNode(rec.taxa_id, -1);
            if ( result == NULL )
                result = dataProvider->root;
        }
        break;
        default:
        {
            throw("Unknow file format");
        }
    }
}

//=========================================================================
void BlastDataTreeLoader::finishProcessing()
{
    dataProvider->updateCache(false);
    LoaderThread::finishProcessing();
}

//=========================================================================
BlastTaxNode::BlastTaxNode(TreeTaxNode *refNode, int _count, BlastNodeMap *blastNodeMap):
    TreeTaxNode(false),
    reads(_count),
    tNode(refNode)
{
    if ( blastNodeMap != NULL )
        blastNodeMap->insert(refNode->getId(), this);
}

//=========================================================================
BlastTaxNode *BlastTaxNode::clone()
{
    BlastTaxNode *n = new BlastTaxNode(tNode, reads, NULL);
    n->collapsed = collapsed;
    n->gnode = NULL;
    n->is_visible = is_visible;
    return n;
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
GraphNode *BlastTaxNode::createGnode(TreeGraphView *gv)
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
