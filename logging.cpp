#include "logging.h"
#include <QDateTime>
#include <QString>
#include <QDir>
Log mlog;

Log::Log()
{
    QString marvaPath = QDir::homePath()+"/Marva/Logs";
    QDir(marvaPath).mkpath(".");
    QDateTime current = QDateTime::currentDateTime();
    file.setFileName(QDir(marvaPath).filePath((QString("marva-%1.log").arg(current.currentMSecsSinceEpoch()))));
}

void Log::log(QString str)
{
    mutex.lock();
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;
    QDateTime current = QDateTime::currentDateTime();
    file.write(QString("%1: %2\n").arg(current.toString()).arg(str).toLocal8Bit());
    file.close();
    mutex.unlock();
}

