#ifndef HISTORY_H
#define HISTORY_H

#include "abstractconfigfile.h"

#include <QObject>

class History : public AbstractConfigFile
{
    Q_OBJECT
    QStringList recentProjects;
public:
    History(QObject *parent=0);
    void addProject(QString &projectName);
    QString lastProject() const;
    virtual void init();
private:
    virtual void toJson(QJsonObject &json) const;
    virtual void fromJson(QJsonObject &json);
public slots:

signals:
    historyChanged();
};

#endif // HISTORY_H
