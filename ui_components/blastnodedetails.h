#ifndef BLASTNODEDETAILS_H
#define BLASTNODEDETAILS_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QFile>

#include "../loader_thread.h"
#include "../blast_record.h"

class BlastTaxNode;
class BlastNodeDetailsModel;
class NodeDetails;


namespace Ui {
class BlastNodeDetails;
}

class BlastNodeDetails : public QDialog
{
    Q_OBJECT
    BlastNodeDetailsModel *model;
    QFile *file;
public:
    explicit BlastNodeDetails(QWidget *parent, BlastTaxNode *n, QString &fileName, BlastFileType type);
    ~BlastNodeDetails();
    inline void setNode(BlastTaxNode *n);

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::BlastNodeDetails *ui;

private slots:
    void refresh();
};

class NodeDetailNode;
typedef QList<NodeDetailNode *> NodeLists;

enum NDNType
{
    ND_BLAST_RECORD        = 0,
    ND_QUERY               = 1,
    ND_NODE_DETAILS        = 2
};

class NodeDetailNode
{
public:
    NodeDetailNode(NDNType t);
    NodeDetailNode *parent;
    NodeLists children;
    virtual QVariant data(int column) = 0;
    int type;
    virtual ~NodeDetailNode();
};

class NDBlastRecord : public NodeDetailNode
{
public:
    NDBlastRecord(BlastRecord *b) : NodeDetailNode(ND_BLAST_RECORD), br(b){}
    virtual QVariant data(int column);
    BlastRecord *br;
    ~NDBlastRecord();
};

class Query : public NodeDetailNode
{
public:
    Query() : NodeDetailNode(ND_QUERY){}
    virtual QVariant data(int column);
    QString name;
    virtual ~Query(){}
};

class NodeDetails : public NodeDetailNode
{
public:
    NodeDetails() : NodeDetailNode(ND_NODE_DETAILS){}
    virtual ~NodeDetails() {}
    virtual QVariant data(int) { return QVariant(); }
    QMap<NDBlastRecord *, Query *> map;
    Query *addQuery(BlastRecord *br);
};

class NodeDetailsLoaderThread : public LoaderThread
{
    BlastTaxNode *node;
    BlastFileType type;
public:
    NodeDetails *nodeDetails;
    NodeDetailsLoaderThread(QObject *parent, QString fileName, BlastTaxNode *_node, BlastFileType _type);
    ~NodeDetailsLoaderThread();
    virtual void run();
    virtual void processLine(QString &line);
};


class BlastNodeDetailsModel : public QAbstractItemModel
{
public:
    BlastTaxNode *node;
    NodeDetails nodeDetails;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    void reset();
};

#endif // BLASTNODEDETAILS_H
