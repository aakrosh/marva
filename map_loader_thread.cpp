#include "map_loader_thread.h"
#include "tax_map.h"
#include "taxdataprovider.h"

//=========================================================================
MapLoaderThread::MapLoaderThread(QObject *parent, bool _merge, GlobalTaxMapDataProvider *_globalTaxDataProvider) :
    LoaderThread(parent, "/data/ncbi.map", "loading taxanomy map", _globalTaxDataProvider, 50000),
    merge(_merge),
    globalTaxDataProvider(_globalTaxDataProvider)
{

}

//=========================================================================
void MapLoaderThread::processLine(QString &line)
{
    QStringList list = line.split("\t", QString::SkipEmptyParts);
    if ( list.size() < 2 )
        throw(QString("Bad .map file record ").append(line).toLocal8Bit().constData());
    qint32 id = list[0].toInt();
    globalTaxDataProvider->taxMap->setName(id, list[1].toLocal8Bit().constData());
}
