#include "history.h"

#include <QDir>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#define CONFIG_FILE_NAME    "marva.hst"
#define MAX_RECENT_PROJ     10

//=========================================================================
History::History()
{
    QString marvaPath = QDir::homePath()+"/Marva";
    QDir(marvaPath).mkpath(".");
    file.setFileName(QDir(marvaPath).filePath(CONFIG_FILE_NAME));
    if ( file.exists() )
        load();
    else
        init();
    connect(this, SIGNAL(historyChanged()), this, SLOT(saveHistory()));
}

//=========================================================================
void History::load()
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
void History::save()
{
    if ( !file.open(QIODevice::WriteOnly) )
    {
        QMessageBox::warning(0, "Failed to save history", QString("Cannot open history file %1").arg(file.fileName()));
        return;
    }
    QJsonObject saveObject;
    toJson(saveObject);
    QJsonDocument saveDoc(saveObject);
    file.write(saveDoc.toJson(QJsonDocument::Indented));
    file.close();
}

//=========================================================================
void History::addProject(QString &projectName)
{
    recentProjects.removeOne(projectName);
    recentProjects.insert(0, projectName);
    while ( recentProjects.size() > MAX_RECENT_PROJ )
        recentProjects.removeLast();
    emit historyChanged();
}

//=========================================================================
QString History::lastProject() const
{
    return recentProjects.size() == 0 ? "" : recentProjects.at(0);
}

//=========================================================================
void History::init()
{
    recentProjects.clear();
}

//=========================================================================
void History::toJson(QJsonObject &json) const
{
    QJsonArray arr;
    foreach(QString proj, recentProjects)
        arr.append(proj);
    json["recentProjects"] = arr;
}

//=========================================================================
void History::fromJson(QJsonObject &json)
{
    QJsonArray arr = json["recentProjects"].toArray();
    recentProjects.clear();
    for ( int  i = 0; i < arr.size(); i++ )
        recentProjects.append(arr[i].toString());
}

//=========================================================================
void History::saveHistory()
{
    save();
}

