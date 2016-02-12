#include "tree_loader_thread.h"
#include "graph_node.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDebug>

/*
Node *TreeLoaderThread::parse(QString &s, Node *parent, int *pos)
{
    if ( s[*pos] == ',')
    {
        (*pos)++;
        return parse(s, parent, pos);
    }
    if ( s[*pos].isDigit() )
    {
        int pstart = *pos;
        while (  s[*pos].isDigit() )
            (*pos)++;
        qint32 tid = s.mid(pstart, *pos-pstart).toInt();
        return parent->addChild(tid);
    }
    else if ( s[(*pos)] == '(' )
    {
        Node *n = parent->addChild(-555);   // Create node with dummy name;
        while ( s[(*pos)] != ')' )
        {
            (*pos)++;         // skip '(' first time and ',' other loops
            parse(s, n, pos);
            if ( s[*pos] == ';' )
                return NULL;
        }
        (*pos)++;
        int pstart = *pos;
        while (  s[*pos].isDigit() )
            (*pos)++;
        qint32 tid = s.mid(pstart, *pos-pstart).toInt();
        n->tax_id = tid;
        return n;
    }
    else if ( s[*pos] == ';' )
    {
        return NULL;
    }
    return parse(s, parent, pos);
}
*/
/*
void mergeNodes(GraphNode *old_node, GraphNode *new_node)
{
    if ( old_node->tax_id != new_node->tax_id )
        return;
    for ( int i = 0; i < new_node->children.count(); i++ )
    {
        GraphNode *nch = new_node->children.at(i);
        for ( int j = 0; j < old_node->children.count(); j++ )
        {
            GraphNode *och = old_node->children.at(j);
            if ( och->tax_id == nch->tax_id )
            {
                mergeNodes(och, nch);
                break;
            }
        }
        old_node->addChild(nch);
        new_node->children.removeAt(i);
    }
}
*/
/*
void TreeLoaderThread::run()
{
    QString fileName = QApplication::applicationDirPath();
    fileName.append("/data/ncbi.tre");
    QFile file(fileName);
    QTextStream in(&file);
    QString line;
    if ( file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        do
        {
            line = in.readLine();
            if ( line == NULL || line.isEmpty() )
                continue;
            Node *dumb = new Node(graph, -666);
            dumb->level--;
            int pos=0;
            parse(line, dumb, &pos);
            Node *newRoot = dumb->children.at(0);
            mergeNodes(graph->root, newRoot);
        }
        while (!line.isNull());
        file.close();
    }
    emit resultReady();
}
*/

TaxNode *TreeLoaderThread::parse(QString &s, TaxNode *parent, int *pos)
{
    if ( s[*pos] == ',')
    {
        (*pos)++;
        return parse(s, parent, pos);
    }
    if ( s[*pos].isDigit() )
    {
        int pstart = *pos;
        while (  s[*pos].isDigit() )
            (*pos)++;
        qint32 tid = s.mid(pstart, *pos-pstart).toInt();
        return parent->addChild(tid);
    }
    else if ( s[(*pos)] == '(' )
    {
        TaxNode *n = parent->addChild(-555);   // Create node with dummy name;
        while ( s[(*pos)] != ')' )
        {
            (*pos)++;         // skip '(' first time and ',' other loops
            parse(s, n, pos);
            if ( s[*pos] == ';' )
                return NULL;
        }
        (*pos)++;
        int pstart = *pos;
        while (  s[*pos].isDigit() )
            (*pos)++;
        qint32 tid = s.mid(pstart, *pos-pstart).toInt();
        n->id = tid;
        return n;
    }
    else if ( s[*pos] == ';' )
    {
        return NULL;
    }
    return parse(s, parent, pos);
}

void TreeLoaderThread::run()
{
    QString fileName = QApplication::applicationDirPath();
    fileName.append("/data/ncbi.tre");
    QFile file(fileName);
    QTextStream in(&file);
    QString line;
    TaxNode *result = NULL;
    if ( file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        do
        {
            line = in.readLine();
            if ( line == NULL || line.isEmpty() )
                continue;

            tree.level = -1;
            int pos=0;
            parse(line, &tree, &pos);
            result = tree.children[0];
        }
        while (!line.isNull());
        file.close();
    }
    emit resultReady(result);
}
