#ifndef LOGGING_H
#define LOGGING_H

#include <QMutex>
#include <QFile>

class Log
{
    QMutex mutex;
    QFile file;
public:
    Log();
    void log(QString str);
};

extern Log mlog;

#endif // LOGGING_H
