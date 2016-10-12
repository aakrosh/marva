#ifndef MAP_LOADER_MAP_H
#define MAP_LOADER_MAP_H

#include "graphview.h"
#include "loader_thread.h"

class GlobalTaxMapDataProvider;

class MapLoaderThread : public LoaderThread
{
    Q_OBJECT
private:
    bool merge;
    GlobalTaxMapDataProvider *globalTaxDataProvider;
    bool must_stop;
public:
    MapLoaderThread(QObject *parent, bool merge, GlobalTaxMapDataProvider *_globalTaxDataProvider);
protected:
    virtual void processLine(QString &line);
protected slots:
    virtual void onFileNameChanged(QString fileName);
};

#endif // MAP_LOADER_MAP_H
