#ifndef TREE_LOADER_THREAD_H
#define TREE_LOADER_THREAD_H

#include "graphview.h"
#include "tax_map.h"
#include "loader_thread.h"

#include <QThread>
#include <QMutex>

extern QMutex treeLoaderMutex;

class GlobalTaxMapDataProvider;

class TreeLoaderThread : public LoaderThread
{
    Q_OBJECT
private:
    bool merge;
    //TaxMap *taxMap;
    GlobalTaxMapDataProvider *dataProvider;
    TaxNode *parse(QString &s, TaxNode *parentTree, int *pos);
public:
    TreeLoaderThread(QObject *parent, GlobalTaxMapDataProvider *_taxMap, bool _merge);
    TaxNode tree;
protected slots:
    virtual void onFileNameChanged(QString fileName);
protected:
    virtual void processLine(QString &line);
    virtual void run();
};

#endif // TREE_LOADER_THREAD_H
