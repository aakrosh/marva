#ifndef TREE_LOADER_THREAD_H
#define TREE_LOADER_THREAD_H

#include "graphwidget.h"
#include "tax_map.h"
#include "loader_thread.h"


#include <QThread>

class TreeLoaderThread : public LoaderThread
{
    Q_OBJECT
private:
    bool merge;
    TaxMap *taxMap;
    TaxNode *parse(QString &s, TaxNode *parentTree, int *pos);
public:
    TreeLoaderThread(QObject *parent, TaxMap *_taxMap, bool _merge) : LoaderThread(parent, "/data/ncbi.tre", "loading taxonomy tree"), merge(_merge), taxMap(_taxMap){}
    TaxNode tree;
protected:
    virtual void processLine(QString &line);
    virtual void finishProcessing();
signals:
    void resultReady(TaxNode *);
};

#endif // TREE_LOADER_THREAD_H
