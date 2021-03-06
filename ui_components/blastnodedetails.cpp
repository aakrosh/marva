#include "blastnodedetails.h"
#include "blast_data.h"
#include "blast_record.h"
#include "ui_blastnodedetails.h"
#include "taxdataprovider.h"
#include "gi2taxmaptxtloader.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#define COLUMN_BLAST_RECORD 1
#define COLUMN_QUERY        0
static int old_row_count = 0;
//=========================================================================
BlastNodeDetails::BlastNodeDetails(QWidget *parent, BlastTaxNode *n, QString &fileName, BlastFileType type) :
    QDialog(parent),
    ui(new Ui::BlastNodeDetails)
{
    old_row_count = 0;
    ui->setupUi(this);
    model = new BlastNodeDetailsModel();
    ui->treeView->setModel(model);
    setNode(n);
    ndlt = new NodeDetailsLoaderThread(this, fileName, n, type);
    ndlt->nodeDetails = &model->nodeDetails;
    connect(ndlt, SIGNAL(finished()), ndlt, SLOT(deleteLater()));
    connect(ndlt, SIGNAL(progress(LoaderThread *, qreal)), this, SLOT(refresh()));
    connect(ndlt, SIGNAL(resultReady(LoaderThread *)), this, SLOT(refresh()));

    connect(this, SIGNAL(finished(int)), ndlt, SLOT(stop_thread()));
    ndlt->start();
}

//=========================================================================
BlastNodeDetails::~BlastNodeDetails()
{
    delete ui;
}

//=========================================================================
void BlastNodeDetails::setNode(BlastTaxNode *n) { model->node = n; }

//=========================================================================
void BlastNodeDetails::closeEvent(QCloseEvent *)
{
    ndlt->Stop();
}

//=========================================================================
void BlastNodeDetails::refresh()
{
    model->reset();
}

//=========================================================================
QModelIndex BlastNodeDetailsModel::index(int row, int column, const QModelIndex &parent) const
{
    if ( !hasIndex(row, column, parent) )
        return QModelIndex();

    if ( !parent.isValid() ) // index of root ( column == 0 )
        return createIndex(row, column, nodeDetails.children.at(row));

    NodeDetailNode* ndn = static_cast<NodeDetailNode*>(parent.internalPointer());
    if ( ndn != NULL )
    {
        NodeDetailNode *child = ndn->children.at(row);
        QModelIndex mi = createIndex(row, column, child);
        return mi;
    }

    return QModelIndex();
}

//=========================================================================
QModelIndex BlastNodeDetailsModel::parent(const QModelIndex &child) const
{
    if ( !child.isValid() )
        return QModelIndex();

    NodeDetailNode *ndn = static_cast<NodeDetailNode *>(child.internalPointer());
    if ( ndn != NULL )
    {
        NodeDetailNode *p = ndn->parent;
        if ( p != &nodeDetails )
        {
            // As we have just 2 hierarchy levels
            // we know that valid parent of item is also a query
            return createIndex(nodeDetails.children.indexOf(p), 0, p);
        }
    }
    return QModelIndex();
}

//=========================================================================
int BlastNodeDetailsModel::rowCount(const QModelIndex &parent) const
{
    if ( parent.column() > 0 )
        return 0;

    if ( parent.isValid() )
    {
        NodeDetailNode *ndn = static_cast<NodeDetailNode *>(parent.internalPointer());
        if ( ndn != NULL )
            return ndn->children.count();
    }
    return nodeDetails.children.count();
}

//=========================================================================
int BlastNodeDetailsModel::columnCount(const QModelIndex &) const
{
    return 10;
}

//=========================================================================
QVariant BlastNodeDetailsModel::data(const QModelIndex &index, int role) const
{
    if ( role == Qt::DisplayRole && index.isValid() )
    {
        NodeDetailNode *ndn = static_cast<NodeDetailNode *>(index.internalPointer());
        return ndn->data(index.column());
    }
    return QVariant();
}

const char * const headers[] =
{
    "",
    "Taxonomy",
    "Identity",
    "Allignment lenght",
    "Mismatches",
    "Gap opens",
    "Query range",
    "Ref range",
    "E value",
    "Bitscore",
};

//=========================================================================
QVariant BlastNodeDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers[section];

    return QVariant();
}

//=========================================================================
void BlastNodeDetailsModel::reset()
{
    //beginResetModel();
    //endResetModel();


    int new_row_count = nodeDetails.children.count();
    if ( new_row_count > old_row_count )
    {
        beginInsertRows(QModelIndex(),old_row_count, new_row_count-1);
        endInsertRows();
        old_row_count = new_row_count;
    }

}

//=========================================================================
NodeDetailsLoaderThread::NodeDetailsLoaderThread(QObject *parent, QString fileName, BlastTaxNode *_node, BlastFileType _type):
    LoaderThread(parent, fileName, "Node details", NULL, 50),
    node(_node),
    type(_type)
{

}

//=========================================================================
NodeDetailsLoaderThread::~NodeDetailsLoaderThread()
{
    if ( gi2TaxProvider != NULL )
    {
        gi2TaxProvider->close();
        delete gi2TaxProvider;
        gi2TaxProvider = NULL;
    }
}

//=========================================================================
void NodeDetailsLoaderThread::run()
{
    QFile file(fileName);
    if ( !file.exists() )
        file.setFileName(QString(QApplication::applicationDirPath()).append(fileName));
    if( !file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        qDebug() << "Cannot open input file " << fileName;
        return;
    }
    int p = 0;

    QTextStream in(&file);
    qint32 nposition = 0;

    if ( type == sequence )
    {
        gi2TaxProvider = new Gi2TaxMapBinProvider(&gi2taxmap);
        gi2TaxProvider->open();
    }
    quint64 fSize = file.size();
    while( !in.atEnd() && nposition < node->positions.size() )
    {
        qint64 pos = node->positions[nposition++];
        in.seek(pos);
        if ( in.atEnd() || must_stop )
            break;
        QString line = in.readLine();
        if ( line == NULL || line.isEmpty() )
            continue;
        processLine(line);
        if ( ++p == progressCounter )
        {
            reportProgress(((qreal)pos)/fSize);
            p = 0;
        }
    }
    file.close();
    if ( !must_stop )
        finishProcessing();
}

//=========================================================================
void NodeDetailsLoaderThread::processLine(QString &line)
{
    BlastRecord *br = new BlastRecord(type, line, false);
    nodeDetails->addQuery(br);
}

//=========================================================================
Query *NodeDetails::addQuery(BlastRecord *br)
{
    Query *q = NULL;
    if ( !children.isEmpty() )
    {
        for(NodeLists::iterator it = children.begin(); it != children.end(); it++ )
        {
            Query *query = (Query *)(*it);
            if ( query->name == br->query_name )
            {
                q = query;
                break;
            }
        }
    }
    if ( q == NULL  )
    {
        children.append(new Query());
        q = (Query *)children.last();
        q->parent = this;
    }
    q->name.swap(br->query_name);
    NDBlastRecord *ndbr = new NDBlastRecord(br);
    q->children.append(ndbr);
    ndbr->parent = q;
    map.insert(ndbr, q);
    return q;
}

//=========================================================================
QVariant NDBlastRecord::data(int column)
{
    switch( column )
    {
        case 0: return br->alligment_id;
        case 1:
            {
                TaxNode *tn = taxMap.value(br->taxa_id);
                return tn == NULL ? "" : tn->getText();
            }
        case 2: return br->identity;
        case 3: return br->allignment_len;
        case 4: return br->mismatch_count;
        case 5: return br->gapopens_count;
        case 6: return QString("%1 - %2").arg(br->query_start).arg(br->query_end);
        case 7: return QString("%1 - %2").arg(br->ref_start).arg(br->ref_end);
        case 8: return br->e_value;
        case 9: return br->bitscore;
    }
    return QVariant();
}

//=========================================================================
NDBlastRecord::~NDBlastRecord()
{
    delete br;
}

//=========================================================================
NodeDetailNode::NodeDetailNode(NDNType t) : parent(NULL), type(t)
{
}

//=========================================================================
NodeDetailNode::~NodeDetailNode()
{
    for(QList<NodeDetailNode *>::iterator it =children.begin(); it != children.end(); it++ )
        delete *it;
}

//=========================================================================
QVariant Query::data(int column)
{
    if ( column == 0 )
        return name;
    return QVariant();
}
