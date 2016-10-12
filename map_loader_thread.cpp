#include "map_loader_thread.h"
#include "tax_map.h"
#include "taxdataprovider.h"

//=========================================================================
MapLoaderThread::MapLoaderThread(QObject *parent, bool _merge, GlobalTaxMapDataProvider *_globalTaxDataProvider) :
    LoaderThread(parent, configuration->Initialization()->taxMapPath(), "loading taxanomy mapping", _globalTaxDataProvider, 50000),
    merge(_merge),
    globalTaxDataProvider(_globalTaxDataProvider)
{

}

//=========================================================================
void MapLoaderThread::onFileNameChanged(QString fileName)
{
    configuration->Initialization()->settaxMapPath(fileName);
    configuration->save();
}

//=========================================================================
void MapLoaderThread::processLine(QString &line)
{
    QStringList list = line.split("\t", QString::SkipEmptyParts);
    if ( list.size() < 2 )
        throw(QString("Bad .map file record ").append(line).toLocal8Bit().constData());    
    globalTaxDataProvider->taxMap->setName(list[0].toInt(), list[1].toLocal8Bit().constData(), (TaxRank)list.last().toInt());
}
