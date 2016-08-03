#ifndef LOADERTHREAD_H
#define LOADERTHREAD_H

#include <QThread>
class QListWidgetItem;

class LoaderThread : public QThread
{
    Q_OBJECT
public:
    LoaderThread(QObject *parent, QString fileName, const char *caption, void *resultObj = NULL, int progressCounter=1000, bool _ignoreRepeated =false, bool trackPosition=false);
    void Stop();
    static void StopRunningThreads();
    virtual void *getResult() { return result; }
protected:
    QString fileName;
    int progressCounter;
    QListWidgetItem *statusListItem;
    bool ignoreRepeated;
    bool trackPosition;
    QString caption;
    void *result;
    bool must_stop;
    quint64 curPos;
    virtual void processLine(QString &line) = 0;
    virtual void finishProcessing();
public:
    virtual void run();
signals:
    void resultReady(LoaderThread *loader);
    void progress(LoaderThread *loader);
private slots:
    void addToList();
    void removeFromList();
    void stop_thread();
};

#endif // LOADERTHREAD_H
