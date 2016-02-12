#include "map_loader_thread.h"
#include "tax_map.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

MapLoaderThread::MapLoaderThread(QObject *parent, bool _merge, GraphView* gw, TaxMap *_map) :
    QThread(parent),
    merge(_merge),
    graph(gw),
    map(_map)
{

}

void MapLoaderThread::run()
{
    QString fileName = QApplication::applicationDirPath();
    fileName.append("/data/ncbi.map");
    QFile file(fileName);
    QTextStream in(&file);
    QString line;
    if ( file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        if ( !merge )
            map->clear();
        int i = 0;
        do
        {
            line = in.readLine();
            if ( line == NULL || line.isEmpty() )
                continue;
            QStringList list = line.split("\t", QString::SkipEmptyParts);
            if ( list.size() < 2 )
                throw(QString("Bad .map file record ").append(line).toLocal8Bit().constData());
            qint32 id = list[0].toInt();
            map->insert(id,list[1]);
            if ( ++i > 1000 )
            {
                progress();
                i = 0;
            }
        }
        while (!line.isNull());
        file.close();
    }/*
    if ( !merge )
    {
        graph->tax_map.swap(new_map);
    }
    else
    {
        TaxMapIterator it = new_map.constEnd();
        const TaxMapIterator b = new_map.constBegin();
        while (it != b)
        {
            --it;
            graph->tax_map.insert(it.key(), it.value());
        }
    }*/
    emit resultReady();
}

