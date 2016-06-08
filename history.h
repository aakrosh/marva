#ifndef HISTORY_H
#define HISTORY_H

#include <QObject>
#include <QFile>

class History : QObject
{
    Q_OBJECT
    QFile file;
    QStringList recentProjects;
public:
    History();
    void load();
    void save();

    void addProject(QString &projectName);
    QString lastProject() const;
private:
    void init();
    void toJson(QJsonObject &json) const;
    void fromJson(QJsonObject &json);
public slots:
    void saveHistory();
signals:
    historyChanged();
};

#endif // HISTORY_H
