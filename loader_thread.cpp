#include "loader_thread.h"
#include "main_window.h"
#include "logging.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>

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
    connect(this, SIGNAL(fileNameChanged(QString)), this, SLOT(onFileNameChanged(QString)));

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
    {
        fileName.prepend(QString(QApplication::applicationDirPath()));
        file.setFileName(fileName);
    }
    bool cannotOpenFile = !file.exists();
    if( cannotOpenFile )
    {
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        QMetaObject::invokeMethod(mainWindow,
                                      "getOpenFileName",
                                      Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(QString , fileName),
                                      Q_ARG(QString, QString("Select file %1").arg(fi.fileName())),
                                      Q_ARG(QString, QString("(*.%1);;All Files(*)").arg(ext))
                                      );
        file.setFileName(fileName);
        cannotOpenFile = fileName.isEmpty();
        if ( !cannotOpenFile )
            emit fileNameChanged(fileName);
    }
    if ( !cannotOpenFile )
    {
        if ( !file.open(QIODevice::ReadOnly|QIODevice::Text) )
        {
            QMetaObject::invokeMethod(mainWindow,
                                      "showMessageBox",
                                      Q_ARG(QString, QString("Connot open file %1").arg(fileName)));
        }
        else
        {
            cannotOpenFile = false;
        }

    }
    if ( cannotOpenFile )
    {
        QMetaObject::invokeMethod(mainWindow, "close");
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

