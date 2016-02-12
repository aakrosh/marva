#ifndef TREE_LOADER_THREAD_H
#define TREE_LOADER_THREAD_H

#include "graphwidget.h"
#include "tax_map.h"


#include <QThread>

class TreeLoaderThread : public QThread
{
    Q_OBJECT
private:
    bool merge;
    TaxNode *parse(QString &s, TaxNode *parentTree, int *pos);
public:
    TreeLoaderThread(QObject *parent, bool _merge) : QThread(parent), merge(_merge){}
    TaxNode tree;
protected:
    virtual void run();
signals:
    void resultReady(TaxNode *);
    void progress(int i);
};

#endif // TREE_LOADER_THREAD_H
