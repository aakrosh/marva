#include "loader_thread.h"
#include "main_window.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDebug>

QList<LoaderThread *> activeThreads;

//=========================================================================
LoaderThread::LoaderThread(QObject *parent, QString _fileName, const char *_caption, void *resultObj, int _progressCounter, bool _ignoreRepeated, bool _trackPosition)
    : QThread(parent),
    fileName(_fileName),
    progressCounter(_progressCounter),
    statusListItem(NULL),
    ignoreRepeated(_ignoreRepeated),
    trackPosition(_trackPosition),
    caption(_caption),
    result(resultObj),
    must_stop(false),
    curPos(-1)
{
    connect(this, SIGNAL(started()), this, SLOT(addToList()));
    connect(this, SIGNAL(finished()), this, SLOT(removeFromList()));
}

//=========================================================================
void LoaderThread::Stop()
{
    must_stop = true;
}

//=========================================================================
void LoaderThread::StopRunningThreads()
{
    foreach (LoaderThread *thread, activeThreads)
        thread->Stop();
}

#define MAX_LINE_SIZE 4096

//=========================================================================
void LoaderThread::run()
{
    QFile file(fileName);
    if ( !file.exists() )
        file.setFileName(QString(QApplication::applicationDirPath()).append(fileName));
    if( !file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        qDebug() << "Cannot open input file " << fileName;
        return;
    }    
    int p = 0;
    QString prevLine;
    if ( trackPosition )
    {
        QTextStream in(&file);
        while( !file.atEnd() )
        {
            if ( must_stop )
                break;
            curPos = file.pos();
            QString line(file.readLine(MAX_LINE_SIZE));
            if ( line == NULL || line.isEmpty() )
                continue;
            if ( ignoreRepeated )
            {
                QString pl = prevLine;
                prevLine = line;
                if ( line == pl )
                    continue;
            }
            processLine(line);
            if ( ++p == progressCounter )
            {
                emit progress(result);
                p = 0;
            }
        }
    }
    else
    {
        QTextStream in(&file);
        while( !in.atEnd() )
        {
            if ( must_stop )
                break;
            QString line = in.readLine();
            if ( line == NULL || line.isEmpty() )
                continue;
            if ( ignoreRepeated )
            {
                QString pl = prevLine;
                prevLine = line;
                if ( line == pl )
                    continue;
            }
            processLine(line);
            if ( ++p == progressCounter )
            {
                emit progress(result);
                p = 0;
            }
        }
    }
    file.close();
    if ( !must_stop )
        finishProcessing();
}

//=========================================================================
void LoaderThread::finishProcessing()
{
    emit resultReady(result);
}

//=========================================================================
void LoaderThread::addToList()
{
    activeThreads.append(this);
    statusListItem = mainWindow->statusList->AddItem(caption);
}

//=========================================================================
void LoaderThread::removeFromList()
{
    activeThreads.removeOne(this);
    mainWindow->statusList->RemoveItem(statusListItem);
    statusListItem = NULL;
}

//=========================================================================
void LoaderThread::stop_thread()
{
    Stop();
}

