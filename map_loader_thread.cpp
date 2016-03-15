#include "map_loader_thread.h"
#include "tax_map.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

//=========================================================================
MapLoaderThread::MapLoaderThread(QObject *parent, bool _merge, GraphView* gw, TaxMap *_map) :
    LoaderThread(parent, "/data/ncbi.map", "loading taxanomy map", NULL, 50000),
    merge(_merge),
    graph(gw),
    map(_map)
{

}

//=========================================================================
void MapLoaderThread::processLine(QString &line)
{
    QStringList list = line.split("\t", QString::SkipEmptyParts);
    if ( list.size() < 2 )
        throw(QString("Bad .map file record ").append(line).toLocal8Bit().constData());
    qint32 id = list[0].toInt();
    map->setName(id, list[1].toLocal8Bit().constData());
}
