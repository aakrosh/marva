#ifndef GI2TAMAPTXTLOADER_H
#define GI2TAMAPTXTLOADER_H

#include "loader_thread.h"

#include <QMap>
#include <QObject>
#include <QFile>

class Gi2TaxMapTxtLoader : public LoaderThread
{
    Q_OBJECT
public:
    QMap<quint32, quint32> *map;
    Gi2TaxMapTxtLoader(QObject *parent, QMap<quint32, quint32> *_map);
protected:
    virtual void processLine(QString &line);
};

class Gi2TaxProvider
{
public:
    QMap<quint32, quint32> *map;
    Gi2TaxProvider(QMap<quint32, quint32> *_map);
    virtual ~Gi2TaxProvider() {}
    virtual void open() {}
    virtual void close() {}
    virtual qint32 get(quint32 gi) = 0;
};


class Gi2TaxMapBinProvider : public Gi2TaxProvider
{
    QString fileName;
    QFile *file;
public:
    Gi2TaxMapBinProvider(QMap<quint32, quint32> *_map);
    virtual ~Gi2TaxMapBinProvider();
    virtual void open();
    virtual void close();
    virtual qint32 get(quint32 gi);
protected:

};

extern Gi2TaxProvider *gi2TaxProvider;
#endif // GI2TAMAPTXTLOADER_H
