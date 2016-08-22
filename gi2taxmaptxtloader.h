#ifndef GI2TAMAPTXTLOADER_H
#define GI2TAMAPTXTLOADER_H

#include "loader_thread.h"

#include <QMap>
#include <QObject>
#include <QFile>
#include <QReadWriteLock>

class Gi2TaxMapTxtLoader : public LoaderThread
{
    Q_OBJECT
public:
    QHash<quint32, quint32> *map;
    Gi2TaxMapTxtLoader(QObject *parent, QHash<quint32, quint32> *_map);
protected:
    virtual void processLine(QString &line);
};

class Gi2TaxProvider
{
public:
    QHash<quint32, quint32> *map;
    Gi2TaxProvider(QHash<quint32, quint32> *_map);
    virtual ~Gi2TaxProvider() {}
    virtual void open() {}
    virtual void close() {}
    virtual qint32 get(quint32 gi) = 0;
};

class RWLocker
{
    QReadWriteLock *l;
public:
    RWLocker(QReadWriteLock *_l, bool read = true) : l (_l)
    {
        if ( read )
            l->lockForRead();
        else
            l->lockForWrite();
    }
    ~RWLocker()
    {
        l->unlock();
    }
};

class Gi2TaxMapBinProvider : public Gi2TaxProvider
{
    QString fileName;
    QFile *file;
    QReadWriteLock lock;
    quint32 fileOpened;
public:
    Gi2TaxMapBinProvider(QHash<quint32, quint32> *_map);
    virtual ~Gi2TaxMapBinProvider();
    virtual void open();
    virtual void close();
    virtual qint32 get(quint32 gi);
protected:

};

extern Gi2TaxProvider *gi2TaxProvider;
#endif // GI2TAMAPTXTLOADER_H
