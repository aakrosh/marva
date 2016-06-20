#include "blastfileloader.h"
#include "blast_data.h"
#include "blast_record.h"
#include "taxdataprovider.h"

#include <QTextStream>
#include <QFileInfo>
#include <exception>
using namespace std;

//=========================================================================
BlastFileLoader::BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp, BlastFileType _type) :
    LoaderThread(parent, fileName, NULL, NULL, 1000, true, true),
    type(_type),
    root(NULL),
    dataProvider(dp)
{
    dp->name = QFileInfo(fileName).fileName();
    caption = QString("Importing blast file: ").append(dp->name);
}

//=========================================================================
BlastFileLoader::~BlastFileLoader()
{
}

//=========================================================================
void BlastFileLoader::addTaxNode(quint32 taxa_id, QList<qint64> pos)
{
    dataProvider->addTaxNode(taxa_id, -1, pos);
    if ( result == NULL )
        result = dataProvider->root;
}

//=========================================================================
void BlastFileLoader::ProcessFinishedQuery()
{
    while ( queryTaxList.count() > 0 )
    {
        quint32 tax_id = queryTaxList[0].tax_id;
        TreeTaxNode *n = globalTaxDataProvider->taxMap->value(tax_id);
        TreeTaxNode *p = n->parent;
        if ( p == NULL )
            break;
        bool moveToParent = true;
        for ( qint32 c = 0 ; moveToParent && (c < p->children.count()); c++ )
            moveToParent = moveToParent && queryTaxList.contains(p->children[c]->getId());
        QList<qint64> positions;
        for ( qint32 c = 0 ; c < p->children.count(); c++ )
        {
            TreeTaxNode *child = p->children[c];
            quint32 id = child->getId();
            qint32 index = queryTaxList.indexOf(id);
            if ( index >= 0 )
            {
                QList<qint64> pos = queryTaxList.at(index).pos;
                queryTaxList.removeAt(index);
                if ( !moveToParent )
                    addTaxNode(id, pos);
                else
                    positions.append(pos);
            }
        }
        if ( moveToParent )
            queryTaxList.append(p->getId(), positions);
    }
}

//=========================================================================
void BlastFileLoader::processLine(QString &line)
{
    switch ( type )
    {
        case tabular:
        {
            QStringList list = line.split("\t", QString::SkipEmptyParts);
            BlastRecord rec(type, list);
            if ( lastQueryName != rec.query_name )
            {
                ProcessFinishedQuery();
                lastQueryName = rec.query_name;
            }
            queryTaxList.append(rec.taxa_id, curPos);
        }
        break;
        default:
        {
            throw("Unknown file format");
        }
    }
}

//=========================================================================
void BlastFileLoader::finishProcessing()
{
    ProcessFinishedQuery();
    dataProvider->updateCache(false);
    LoaderThread::finishProcessing();
}


