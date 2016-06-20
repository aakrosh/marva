#include "history.h"

#include <QDir>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#define MAX_RECENT_PROJ     10

//=========================================================================
History::History(QObject *parent) : AbstractConfigFile("hst", "history", parent)
{
}

//=========================================================================
void History::addProject(QString &projectName)
{
    recentProjects.removeOne(projectName);
    recentProjects.insert(0, projectName);
    while ( recentProjects.size() > MAX_RECENT_PROJ )
        recentProjects.removeLast();
    save();
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
    try
    {
        QJsonArray arr = json["recentProjects"].toArray();
        recentProjects.clear();
        for ( int  i = 0; i < arr.size(); i++ )
            recentProjects.append(arr[i].toString());
    }
    catch (...)
    {
        QMessageBox::warning(NULL, "Error occured", "Cannot restore history from histore file");
    }
}

