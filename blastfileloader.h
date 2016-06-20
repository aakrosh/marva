#ifndef BLASTFILELOADER_H
#define BLASTFILELOADER_H

#include "loader_thread.h"
#include "blast_data.h"

#include <QList>
#include <QMap>

struct TaxPos
{
   quint32 tax_id;
   QList<qint64> pos;
   TaxPos(quint32 t, qint64 p):tax_id(t) {pos.append(p);}
   TaxPos(quint32 t, QList<qint64> _pos):tax_id(t), pos(_pos){}
   inline bool operator==(const TaxPos &otherTaxPos) { return tax_id == otherTaxPos.tax_id; }
};

class TaxPosList : public QList<TaxPos>
{
public:
    bool contains(quint32 id)
    {
        TaxPos q(id, 0);
        return QList<TaxPos>::contains(q);
    }
    int indexOf(quint32 id)
    {
        TaxPos q(id, 0);
        return QList<TaxPos>::indexOf(q);
    }
    void append(quint32 id, QList<qint64> positions)
    {
        TaxPos q(id, positions);
        QList<TaxPos>::append(q);
    }
    void append(quint32 id, qint64 position)
    {
        TaxPos q(id, position);
        QList<TaxPos>::append(q);
    }
};

typedef QList<quint32> TaxList;
typedef QMap<QString, TaxList> AllignmentMap;

class BlastFileLoader: public LoaderThread
{
private:
    BlastFileType type;
    BlastTaxNode *root;
    BlastTaxDataProvider *dataProvider;
    AllignmentMap allignment_tax_ids;
    QString lastQueryName;
    TaxPosList queryTaxList;

    void ProcessFinishedQuery();
    void addTaxNode(quint32 taxa_id, QList<qint64> pos);
public:
    BlastFileLoader(QObject *parent, QString fileName, BlastTaxDataProvider *dp, BlastFileType _type);
    ~BlastFileLoader();

protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();

};


#endif // BLASTFILELOADER_H
