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
    TreeLoaderThread(QObject *parent, TaxMap *_taxMap, bool _merge);
    TaxNode tree;
protected:
    virtual void processLine(QString &line);
};

#endif // TREE_LOADER_THREAD_H
