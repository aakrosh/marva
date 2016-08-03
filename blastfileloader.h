#ifndef BLASTFILELOADER_H
#define BLASTFILELOADER_H

#include "loader_thread.h"
#include "blast_data.h"

#include <QList>
#include <QVector>
#include <QMap>

struct TaxPos
{
   qint32 tax_id;
   qreal max_score;
   QVector<qint64> pos;
   TaxPos(qint32 t, qint64 p):tax_id(t) {pos.append(p);}
   TaxPos(qint32 t, QVector<qint64> _pos):tax_id(t), pos(_pos){}
   inline bool operator==(const TaxPos &otherTaxPos) { return tax_id == otherTaxPos.tax_id; }
};

class TaxPosList : public QList<TaxPos>
{
public:
    bool contains(qint32 id)
    {
        TaxPos q(id, 0);
        return QList<TaxPos>::contains(q);
    }
    int indexOf(qint32 id)
    {
        TaxPos q(id, 0);
        return QList<TaxPos>::indexOf(q);
    }
    void append(qint32 id, QVector<qint64> positions)
    {
        TaxPos q(id, positions);
        QList<TaxPos>::append(q);
    }
    void append(qint32 id, qint64 position)
    {
        TaxPos q(id, position);
        QList<TaxPos>::append(q);
    }
};

typedef QList<quint32> TaxList;
typedef QMap<QString, TaxList> AllignmentMap;

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
    AllignmentMap allignment_tax_ids;
    QString lastQueryName;
    TaxPosList queryTaxList;

    void ProcessFinishedQuery();
    void addTaxNode(quint32 taxa_id, QVector<qint64> pos);
public:
    BlastFileType type;
    BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp);
    ~BlastFileLoader();

protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();
};


#endif // BLASTFILELOADER_H
