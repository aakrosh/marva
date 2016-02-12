#ifndef MAP_LOADER_MAP_H
#define MAP_LOADER_MAP_H

#include "graphwidget.h"

#include <QThread>

class MapLoaderThread : public QThread
{
    Q_OBJECT
private:
    bool merge;
    GraphView *graph;
    TaxMap *map;
public:
    MapLoaderThread(QObject *parent, bool merge, GraphView *gw, TaxMap *map);
protected:
    virtual void run();
signals:
    void resultReady();
    void progress();
};

#endif // MAP_LOADER_MAP_H
