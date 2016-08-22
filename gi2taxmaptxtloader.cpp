#include "gi2taxmaptxtloader.h"
#include "config.h"
#include "main_window.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
Gi2TaxProvider *gi2TaxProvider = NULL;

//=========================================================================
Gi2TaxMapTxtLoader::Gi2TaxMapTxtLoader(QObject *parent, QHash<quint32, quint32> *_map):
    LoaderThread(parent, "/data/gi2tax.map", "loading gi to taxonomy mapping", _map, 1000),
    map(_map)
{

}

//=========================================================================
void Gi2TaxMapTxtLoader::processLine(QString &line)
{
    QStringList lines = line.split(' ');
    if ( lines.size() < 2 )
        throw(QString("Bad taxonomy mapping file record ").append(line).toLocal8Bit().constData());
    map->insert(lines[0].toUInt(), lines[1].toUInt());
}

//=========================================================================
Gi2TaxMapBinProvider::Gi2TaxMapBinProvider(QHash<quint32, quint32> *_map):
    Gi2TaxProvider(_map),
    fileName("/data/gi2tax.bin"),
    fileOpened(false)
{

}

Gi2TaxMapBinProvider::~Gi2TaxMapBinProvider()
{
}

//=========================================================================
void Gi2TaxMapBinProvider::open()
{
    {
        RWLocker l(&lock, false);
        if ( fileOpened++ > 0 )
            return;
    }
    if ( !configuration->Paths()->gi2taxmap().isEmpty() )
        fileName = configuration->Paths()->gi2taxmap();
    file = new QFile(fileName);
    if ( !file->exists() )
        file->setFileName(QString(QApplication::applicationDirPath()).append(fileName));
    if ( !file->exists() )
    {
            QMetaObject::invokeMethod(mainWindow,
                                      "getOpenFileName",
                                      Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(QString , fileName),
                                      Q_ARG(QString, "Select GI to Taxonomy mapping file"),
                                      Q_ARG(QString, "(*.bin *.map *.txt)")
                                      );
        if ( fileName.isEmpty() )
            return;
        configuration->Paths()->setgi2taxmap(fileName);
        file->setFileName(fileName);
    }
    if( !file->open(QIODevice::ReadOnly|QIODevice::Unbuffered) )
    {
        qDebug() << "Cannot open input file " << file->fileName();
        return;
    }
}

//=========================================================================
void Gi2TaxMapBinProvider::close()
{
    if ( --fileOpened == 0 )
    {
        file->close();
        delete file;
    }
}

//=========================================================================
qint32 Gi2TaxMapBinProvider::get(quint32 gi)
{
    {
        RWLocker locker(&lock);
        if ( map->contains(gi) )
            return map->value(gi);
    }
    qulonglong lgi = gi;
    lgi = lgi << 2;
    RWLocker locker(&lock, false);
    if ( file->seek(lgi) )
    {
        uchar buf[4];
        if ( file->read((char *)&buf, sizeof(buf)) == sizeof(buf) )
        {
            quint32 val = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
            map->insert(gi, val);
            return val;
        }
    }
    return -2;
}


//=========================================================================
Gi2TaxProvider::Gi2TaxProvider(QHash<quint32, quint32> *_map)
    : map(_map)
{

}
