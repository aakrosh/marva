#include "loader_thread.h"
#include "main_window.h"
#include "logging.h"

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
    connect(this, SIGNAL(progress(LoaderThread* ,qreal)), this, SLOT(onProgress(LoaderThread* ,qreal)));
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
    quint64 fileSize = file.size();
    if ( trackPosition )
    {
        while( !file.atEnd() )
        {
            if ( must_stop )
                break;
            curPos = file.pos();
            char buf[MAX_LINE_SIZE];
            if ( file.readLine(buf, MAX_LINE_SIZE) == 0 )
                continue;
            QString line(buf);
            if ( line.isEmpty() )
                continue;
            if ( ignoreRepeated )
            {
                if ( line == prevLine )
                    continue;
                prevLine = line;
            }
            processLine(line);
            if ( ++p == progressCounter )
            {
                reportProgress(((qreal)curPos)/fileSize);
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
                reportProgress(-1);
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
    emit resultReady(this);
}

//=========================================================================
void LoaderThread::reportProgress(qreal val)
{
    emit progress(this, val);
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
    statusListItem->setText(caption);
    mainWindow->statusList->RemoveItem(statusListItem);     
    statusListItem = NULL;
}

//=========================================================================
void LoaderThread::onProgress(LoaderThread *, qreal val)
{    
    if ( val < 0 )
        return;
    int percents = 100 * val;
    /*QString txt = statusListItem->text();
    int b = txt.indexOf(" ( ");
    if ( b > 0 )
        txt = txt.left(b);
    txt.append(QString(" ( %1% )").arg(percents));
    */
    QString txt = QString("%1 (%2%)").arg(caption).arg(percents);
    if ( statusListItem != NULL )
        statusListItem->setText(txt);
    mlog.log(txt);
}

//=========================================================================
void LoaderThread::stop_thread()
{
    Stop();
}

