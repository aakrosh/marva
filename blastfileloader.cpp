#include "blastfileloader.h"
#include "blast_data.h"
#include "blast_record.h"
#include "taxdataprovider.h"
#include "gi2taxmaptxtloader.h"
#include "blastfileloader.h"

#include <QTextStream>
#include <QFileInfo>
#include <exception>
#include <QDebug>

using namespace std;

//=========================================================================
BlastFileLoader::BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp) :
    LoaderThread(parent, fileName, NULL, NULL, 1000, true, true),
    parser(NULL),
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
void BlastFileLoader::addTaxNode(quint32 taxa_id, QVector<qint64> pos)
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
        TaxPos &tp = queryTaxList.first();
        quint32 tax_id = tp.tax_id;
        TreeTaxNode *n = globalTaxDataProvider->taxMap->value(tax_id);
        if ( n == NULL )
        {
            qDebug() << QString("Taxonomy id %1 is unknown").arg(tax_id);
            queryTaxList.removeFirst();
            continue;
        }
        TreeTaxNode *p = n->parent;

        quint32 ch_count = p == NULL ? 0 : p->children.count();
        bool moveToParent = ch_count > 1;
        for ( qint32 c = 0 ; moveToParent && (c < p->children.count()); c++ )
            moveToParent = moveToParent && queryTaxList.contains(p->children[c]->getId());
        if ( moveToParent )
        {
            QVector<qint64> positions;
            for ( qint32 c = 0 ; c < p->children.count(); c++ )
            {
                TreeTaxNode *child = p->children[c];
                quint32 id = child->getId();
                qint32 index = queryTaxList.indexOf(id);
                if ( index >= 0 )
                {
                    positions.append(queryTaxList.at(index).pos);
                    queryTaxList.removeAt(index);
                }
            }
            queryTaxList.append(p->getId(), positions);
        }
        else
        {
            addTaxNode(tax_id, tp.pos);
            queryTaxList.removeFirst();
        }
    }
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
    BlastRecord *rec = parser->parse(line);
    if ( rec == NULL )
        return;
    if ( lastQueryName != rec->query_name )
    {
        ProcessFinishedQuery();
        lastQueryName = rec->query_name;
    }
    queryTaxList.append(rec->taxa_id, curPos);
    delete rec;
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
SequenceBlastParser::SequenceBlastParser():
    BlastParser()
{
    gi2TaxProvider = new Gi2TaxMapBinProvider(&gi2taxmap);
    gi2TaxProvider->open();
}

SequenceBlastParser::~SequenceBlastParser()
{
    gi2TaxProvider->close();
    delete gi2TaxProvider;
    gi2TaxProvider = NULL;
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
