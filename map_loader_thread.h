#ifndef MAP_LOADER_MAP_H
#define MAP_LOADER_MAP_H

#include "graphwidget.h"
#include "loader_thread.h"


class MapLoaderThread : public LoaderThread
{
    Q_OBJECT
private:
    bool merge;
    GraphView *graph;
    TaxMap *map;
    bool must_stop;
public:
    MapLoaderThread(QObject *parent, bool merge, GraphView *gw, TaxMap *map);
protected:
    virtual void processLine(QString &line);
};

#endif // MAP_LOADER_MAP_H
