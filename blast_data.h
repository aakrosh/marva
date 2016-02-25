#ifndef BLAST_DATA
#define BLAST_DATA

#include "blast_record.h"
#include "loader_thread.h"
#include "tax_map.h"


#include <QList>
#include <QMap>

typedef QMap<quint32, quint32> TaxQuantity;
typedef QMap<qint32, quint32> TaxColorMap;


class TaxColorSrc : public TaxColorMap
{
public:
    virtual quint32 getColor(qint32 tax_id);
};
extern TaxColorSrc colorSrc;

class BlastTaxNode : public BaseTaxNode
{
public:
    int count;
    TaxNode *tNode;
    BlastTaxNode(TaxNode *refNode, int _count);
    virtual ~BlastTaxNode(){}
    BlastTaxNode *createPathToNode();

    virtual QString getName() { return tNode->getName(); }
    virtual qint32 getId() { return tNode->getId(); }
    virtual int getLevel() { return tNode->getLevel(); }
    virtual void setLevel(int) { } //The level of tNode is already set and could not be modified
    // TODO: Fix me
    virtual GraphNode *createGnode(GraphView *gv);
    virtual QString getText() { return tNode->getText(); }
};

typedef QMap<qint32, BlastTaxNode *> BlastNodeMap;
extern BlastNodeMap blastNodeMap;

class BlastDataTreeLoader: public LoaderThread
{
private:
    BlastFileType type;
    BlastTaxNode *root;
public:
    BlastDataTreeLoader(QObject *parent, QString fileName, BlastFileType _type);
protected:
    virtual void processLine(QString &line);

};

#endif // BLAST_DATA

