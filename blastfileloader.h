#ifndef BLASTFILELOADER_H
#define BLASTFILELOADER_H

#include "loader_thread.h"
#include "blast_data.h"

#include <QList>
#include <QVector>
#include <QMap>

class QFile;

class BlastParser
{
public:
    virtual ~BlastParser() {}
    virtual BlastFileType getType() = 0;
    virtual bool accept(QString &line) = 0;
    virtual BlastRecord *parse(QString &line);
protected:
    bool openFile();
    bool closeFile();
};

class TabSeparatedBlastParser : public BlastParser
{
public:
    inline virtual BlastFileType getType() { return tabular; }
    virtual bool accept(QString &line);
};

extern QMap<quint32, quint32> gi2taxmap;
class SequenceBlastParser : public BlastParser
{
public:
    SequenceBlastParser();
    ~SequenceBlastParser();
    inline virtual BlastFileType getType() { return sequence; }
    virtual bool accept(QString &line);
    virtual BlastRecord *parse(QString &line);
};

#include <QDebug>

class TaxDetails
{
public:
    qreal   score;
    qreal   e_value;
    quint64 pos;
    TaxDetails( qreal s, qreal e, quint64 p) : score(s), e_value(e), pos(p){}
    ~TaxDetails()
    {
        qDebug() << "Destructor is called";
    }
};

class TaxDetailsMap : public QMap<qint32, TaxDetails *>
{
public:
    void add(BlastRecord *br, quint64 pos)
    {
        if ( contains(br->taxa_id) )
        {
            TaxDetails *td = value(br->taxa_id);
            if ( td->score > br->bitscore )
                return;
            td->score = br->bitscore;
            td->pos = pos;
            td->e_value = br->e_value;
        }
        else
        {
            insert(br->taxa_id, new TaxDetails(br->bitscore, br->e_value, pos));
        }
    }
    void positions(QVector<quint64> *positions)
    {
        QList<TaxDetails *> vals = values();

        for ( int i = 0; i < vals.count(); i++ )
            positions->append(vals[i]->pos);
    }
};

class QueryDetails
{
public:
    QString queryName;
    TaxDetailsMap tax_details_map;
    void clean();
    void add(QString newQueryName, BlastRecord *br, quint32 pos);

    quint32 lca_id();
};

class BlastParserFactory
{
public:
    static BlastParser *createParser(BlastFileType type)
    {
        switch (type)
        {
            case tabular:
                return new TabSeparatedBlastParser();
            case sequence:
                return new SequenceBlastParser();
            default:
                break;
        }
        return NULL;
    }
};

class BlastFileLoader: public LoaderThread
{
private:
    BlastParser *parser;
    BlastTaxNode *root;
    BlastTaxDataProvider *dataProvider;
    QueryDetails lastQuery;

    void ProcessFinishedQuery();
    void addTaxNode(quint32 taxa_id, QVector<quint64> pos);
public:
    BlastFileType type;
    BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp);
    ~BlastFileLoader();

protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();
};


#endif // BLASTFILELOADER_H
