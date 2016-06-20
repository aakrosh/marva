#include "abstractconfigfile.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

//=========================================================================
AbstractConfigFile::AbstractConfigFile(QString extension, QString _caption, QObject *parent) :
    QObject(parent),
    caption(_caption)
{
    QString marvaPath = QDir::homePath()+"/Marva";
    QDir(marvaPath).mkpath(".");
    file.setFileName(QDir(marvaPath).filePath(QString("marva.").append(extension)));
}

//=========================================================================
AbstractConfigFile::~AbstractConfigFile()
{
    QList<AbstractConfigBlock *> values = blocks.values();
    foreach(AbstractConfigBlock *acb, values)
    {
       delete acb;
    }
}

//=========================================================================
void AbstractConfigFile::init()
{
    QList<AbstractConfigBlock *> values = blocks.values();
    foreach(AbstractConfigBlock *acb, values)
    {
       connect(acb, SIGNAL(dataChanged()), this, SLOT(save()));
    }
}

//=========================================================================
void AbstractConfigFile::load()
{
    if ( !file.open(QIODevice::ReadOnly) )
        return;
    QByteArray saveData = file.readAll();
    file.close();
    QJsonDocument loadDoc = QJsonDocument::fromJson(saveData);
    QJsonObject jobj = loadDoc.object();
    fromJson(jobj);
}

//=========================================================================
void AbstractConfigFile::save()
{
    if ( !file.open(QIODevice::WriteOnly) )
    {
        QMessageBox::warning(0, QString("Failed to save %1").arg(caption), QString("Cannot open %1 file %2").arg(caption).arg(file.fileName()));
        return;
    }
    QJsonObject saveObject;
    toJson(saveObject);
    QJsonDocument saveDoc(saveObject);
    file.write(saveDoc.toJson(QJsonDocument::Indented));
    file.close();
}

//=========================================================================
void AbstractConfigFile::toJson(QJsonObject &json) const
{
    QList<AbstractConfigBlock *> values = blocks.values();
    foreach(AbstractConfigBlock *acb, values)
    {
        acb->toJson(json);
    }
}

//=========================================================================
void AbstractConfigFile::fromJson(QJsonObject &json)
{
    QList<AbstractConfigBlock *> values = blocks.values();
    if ( json.count() < values.count() )
        return;
    foreach(AbstractConfigBlock *acb, values)
    {
        acb->fromJson(json);
    }
}

