#include "loader_thread.h"
#include "main_window.h"

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QMessageBox>

QList<LoaderThread *> activeThreads;

//=========================================================================
LoaderThread::LoaderThread(QObject *parent, QString _fileName, const char *_caption, void *resultObj, int _progressCounter)
    : QThread(parent),
    fileName(_fileName),
    progressCounter(_progressCounter),
    caption(_caption),
    statusListItem(NULL),
    result(resultObj),
    must_stop(false)
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

//=========================================================================
void LoaderThread::run()
{
    QFile file(fileName);
    if ( !file.exists() )
        file.setFileName(QString(QApplication::applicationDirPath()).append(fileName));
    if( !file.open(QIODevice::ReadOnly|QIODevice::Text) )
    {
        QMessageBox::information(0,"error",file.errorString());
        return;
    }
    QTextStream in(&file);
    int p = 0;
    while( !in.atEnd() )
    {
        if ( must_stop )
            break;
        QString line = in.readLine();
        if ( line == NULL || line.isEmpty() )
            continue;        
        processLine(line);
        if ( ++p == progressCounter )
        {
            emit progress(result);
            p = 0;
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
    statusListItem=NULL;
}

