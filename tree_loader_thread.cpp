#include "tree_loader_thread.h"
#include "graph_node.h"

//=========================================================================
TaxNode *TreeLoaderThread::parse(QString &s, TaxNode *parent, int *pos)
{
    if ( must_stop )
        return NULL;
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
        TaxNode *newNode = (TaxNode *)parent->addChildById(tid);
        taxMap->insert(newNode->id, newNode);
        return newNode;
    }
    else if ( s[(*pos)] == '(' )
    {
        TaxNode *n = (TaxNode *)parent->addChildById(-555);   // Create node with dummy name;
        if ( must_stop || n == NULL )
            return NULL;
        while ( s[(*pos)] != ')' )
        {
            if ( must_stop )
                return NULL;
            (*pos)++;         // skip '(' first time and ',' other loops
            parse(s, n, pos);
            if ( s[*pos] == ';' || must_stop )
                return NULL;
        }
        (*pos)++;
        int pstart = *pos;
        while (  s[*pos].isDigit() )
            (*pos)++;
        qint32 tid = s.mid(pstart, *pos-pstart).toInt();
        n->id = tid;
        taxMap->insert(tid, n);
        return n;
    }
    else if ( s[*pos] == ';' )
    {
        return NULL;
    }
    return parse(s, parent, pos);
}

//=========================================================================
void TreeLoaderThread::processLine(QString &line)
{
    tree.level = -1;
    int pos=0;
    parse(line, &tree, &pos);
}

//=========================================================================
void TreeLoaderThread::finishProcessing()
{
    emit resultReady(tree.children.size() == 0 ? NULL : (TaxNode *)tree.children[0]);
}
