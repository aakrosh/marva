#include "tree_loader_thread.h"
#include "graph_node.h"

//=========================================================================
TreeLoaderThread::TreeLoaderThread(QObject *parent, TaxMap *_taxMap, bool _merge) :
    LoaderThread(parent, "/data/ncbi.tre", "loading taxonomy tree"),
    merge(_merge),
    taxMap(_taxMap)
{
}

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
        if ( taxMap->contains(tid) )
            return taxMap->value(tid);
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
        if ( taxMap->contains(tid) )
        {
            TaxNode *tn = taxMap->value(tid);
            for (ChildrenList::ConstIterator it = n->children.constBegin(); it != n->children.constEnd(); it++ )
            {
                if ( !tn->children.contains(*it) )
                    tn->addChild(*it);
                else
                    (*it)->parent = tn;
            }
            parent->children.removeOne(n);
            parent->addChild(tn);
            delete n;
            return tn;
        }
        else
        {
            n->id = tid;
            taxMap->insert(tid, n);
        }
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
    result = tree.children.size() == 0 ? NULL : (TaxNode *)tree.children[0];
    if ( result != NULL )
        ((BaseTaxNode *)result)->parent = NULL;
}
