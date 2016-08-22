#include "blastfileloader.h"
#include "blast_data.h"
#include "blast_record.h"
#include "taxdataprovider.h"
#include "gi2taxmaptxtloader.h"
#include "blastfileloader.h"
#include "taxdataprovider.h"

#include <QTextStream>
#include <QFileInfo>
#include <exception>
#include <QDebug>

using namespace std;

//=========================================================================
BlastFileLoader::BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp) :
    LoaderThread(parent, fileName, NULL, NULL, 6000, true, true),
    parser(NULL),
    root(NULL),
    dataProvider(dp),
    oldCount(0)
{
    dp->name = QFileInfo(fileName).fileName();
    caption = QString("Importing blast file: ").append(dp->name);
}

//=========================================================================
BlastFileLoader::~BlastFileLoader()
{
}

//=========================================================================
void BlastFileLoader::addTaxNode(quint32 taxa_id, QVector<quint64> &pos)
{
    dataProvider->addTaxNode(taxa_id, -1, pos);
    if ( result == NULL )
        result = dataProvider->root;
}

//=========================================================================
void BlastFileLoader::ProcessFinishedQuery()
{
    if ( lastQuery.tax_details_map.empty() )
        return;
    // Save positions
    QVector<quint64> &poss = lastQuery.tax_details_map.positions();
    // Find LCA
    qint32 lca_id = lastQuery.lca_id();
    addTaxNode(lca_id, poss);
    lastQuery.clean();
}

//=========================================================================
void BlastFileLoader::processLine(QString &line)
{
    if ( parser == NULL )
    {
        for ( int t = (int)tabular; t < (int)last; ++t )
        {
            parser = BlastParserFactory::createParser((BlastFileType)t);
            if ( parser->accept(line) )
            {
                type = parser->getType();
                break;
            }
            delete parser;
            parser = NULL;
        }
    }
    if ( parser == NULL )
        return;
    BlastRecord rec;
    //BlastRecord *rec = parser->parse(line);
    parser->parse(line, rec);
//    if ( rec == NULL )
//        return;
    if ( rec.bitscore < 70 )           // TODO: Configure
        return;
    if ( lastQuery.queryName != rec.query_name )
        ProcessFinishedQuery();
    lastQuery.add(rec.query_name, &rec, curPos);
//    delete rec;
}

//=========================================================================
void BlastFileLoader::finishProcessing()
{
    ProcessFinishedQuery();
    dataProvider->updateCache(false);
    if ( parser != NULL )
        delete parser;
    LoaderThread::finishProcessing();
}

//=========================================================================
void BlastFileLoader::reportProgress(qreal val)
{
    updateDataProviderCache();
    LoaderThread::reportProgress(val);
}

//=========================================================================
void BlastFileLoader::updateDataProviderCache()
{
    quint32 newCount = dataProvider->count();
    dataProvider->updateCache(oldCount != newCount);
    oldCount = newCount;
}

//=========================================================================
bool TabSeparatedBlastParser::accept(QString &line)
{
    return line.count('\t') == 12;
}

//=========================================================================
BlastRecord *BlastParser::parse(QString &line)
{
    return new BlastRecord(getType(), line);
}

//=========================================================================
void BlastParser::parse(QString &line, BlastRecord &rec)
{
    r.parse(getType(), line, rec);
}


//=========================================================================
SequenceBlastParser::SequenceBlastParser():
    BlastParser()
{
    if ( gi2TaxProvider == NULL )
        gi2TaxProvider = new Gi2TaxMapBinProvider(&gi2taxmap);
    gi2TaxProvider->open();
}

//=========================================================================
SequenceBlastParser::~SequenceBlastParser()
{
    gi2TaxProvider->close();
}

//=========================================================================
bool SequenceBlastParser::accept(QString &line)
{
    return line.at(0) == '>' && line.count(' ') == 9;
}

//=========================================================================
BlastRecord *SequenceBlastParser::parse(QString &line)
{
    if ( !accept(line) )
        return NULL;
    return BlastParser::parse(line);
}

//=========================================================================
void QueryDetails::clean()
{
    queryName.clear();
    tax_details_map.clean();
}

//=========================================================================
void QueryDetails::add(QString newQueryName, BlastRecord *br, quint32 pos)
{
    queryName = newQueryName;
    tax_details_map.add(br, pos);
}

//=========================================================================
quint32 QueryDetails::lca_id()
{
    QList<qint32> tax_ids = tax_details_map.keys();
    qint32 i = 0;
    qint32 cur_id = tax_ids[i++];
    TaxNode *cur_node = globalTaxDataProvider->taxMap->value(cur_id);
    while ( ( cur_id < 0 || cur_node == NULL ) && i < tax_ids.count() )
    {
        cur_id = tax_ids[i++];
        if ( cur_id < 0 )
            continue;
        cur_node = globalTaxDataProvider->taxMap->value(cur_id);
    }
    while ( i < tax_ids.count() )
    {
        qint32  next_id = tax_ids[i++];
        if ( next_id < 0 )
            continue;
        TaxNode *next_node = globalTaxDataProvider->taxMap->value(next_id);
        if ( next_node == NULL )
            continue;
        while ( next_node->getLevel() > cur_node->getLevel() )
            next_node = (TaxNode *)next_node->parent;
        while ( next_node->getLevel() < cur_node->getLevel() )
            cur_node = (TaxNode *)cur_node->parent;
        while ( next_node != cur_node )
        {
            next_node = (TaxNode *)next_node->parent;
            cur_node = (TaxNode *)cur_node->parent;
        }
    }
    return cur_node == NULL ? -2 : cur_node->getId();
}
