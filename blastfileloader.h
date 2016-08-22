#ifndef BLASTFILELOADER_H
#define BLASTFILELOADER_H

#include "loader_thread.h"
#include "blast_data.h"

#include <QList>
#include <QVector>
#include <QHash>

class QFile;

class BlastParser
{
    BlastRecord r;
public:
    virtual ~BlastParser() {}
    virtual BlastFileType getType() = 0;
    virtual bool accept(QString &line) = 0;
    virtual BlastRecord *parse(QString &line);
    virtual void parse(QString &line, BlastRecord &rec);
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

extern QHash<quint32, quint32> gi2taxmap;
class SequenceBlastParser : public BlastParser
{
public:
    SequenceBlastParser();
    ~SequenceBlastParser();
    inline virtual BlastFileType getType() { return sequence; }
    virtual bool accept(QString &line);
    virtual BlastRecord *parse(QString &line);
};

class TaxDetails
{
public:
    qreal   score;
    qreal   e_value;
    quint64 *pos;
    TaxDetails( qreal s, qreal e, quint64 *p) : score(s), e_value(e), pos(p){}
    TaxDetails() {}
};

class TaxDetailsMap : public QHash<qint32, TaxDetails *>
{
#define TDARRAY_SIZE    1024
    TaxDetails tdArray[TDARRAY_SIZE];
    QVector<TaxDetails *> tdVector;
    quint32 curtd;
    QVector<quint64> posvec;
    quint32 posvec_size;
public:
    TaxDetailsMap() : curtd(0), posvec_size(0){ }
    void add(BlastRecord *br, quint64 pos)
    {
        if ( contains(br->taxa_id) )
        {
            TaxDetails *td = value(br->taxa_id);
            if ( td->score > br->bitscore )
                return;
            td->score = br->bitscore;
            *(td->pos) = pos;
            td->e_value = br->e_value;
        }
        else
        {
            if ( posvec_size <= curtd )
            {
                posvec_size += TDARRAY_SIZE;
                posvec.resize(posvec_size);
            }
            if ( curtd < TDARRAY_SIZE )
            {
                TaxDetails *td = &tdArray[curtd];
                td->e_value = br->e_value;
                td->score = br->bitscore;
                td->pos = &posvec[curtd];
                *(td->pos) = pos;

                insert(br->taxa_id, td);
            }
            else
            {
                posvec[curtd] = pos;
                TaxDetails *td = new TaxDetails(br->bitscore, br->e_value, &posvec[curtd]);
                insert(br->taxa_id, td);
                tdVector.append(td);
            }
            curtd++;
        }
    }
    QVector<quint64> &positions()
    {
        posvec.resize(curtd);
        return posvec;
    }
    void clean()
    {
        if ( curtd >= TDARRAY_SIZE )
        {
            qDeleteAll(tdVector);
            tdVector.clear();
        }
        clear();
        curtd = 0;
        posvec_size = 0;
    }
};

class QueryDetails
{
public:
    QueryDetails() {}
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
    quint32 oldCount;

    void ProcessFinishedQuery();
    void addTaxNode(quint32 taxa_id, QVector<quint64> &pos);
public:
    BlastFileType type;
    BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp);
    ~BlastFileLoader();

protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();
    virtual void reportProgress(qreal val);

public slots:
    virtual void updateDataProviderCache();
};


#endif // BLASTFILELOADER_H
