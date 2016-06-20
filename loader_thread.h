#ifndef LOADERTHREAD_H
#define LOADERTHREAD_H

#include <QThread>
class QListWidgetItem;

class LoaderThread : public QThread
{
    Q_OBJECT
private:
    QString fileName;
    int progressCounter;
    QListWidgetItem *statusListItem;
    bool ignoreRepeated;
    bool trackPosition;
public:
    LoaderThread(QObject *parent, QString fileName, const char *caption, void *resultObj = NULL, int progressCounter=1000, bool _ignoreRepeated =false, bool trackPosition=false);
    void Stop();
    static void StopRunningThreads();
protected:
    QString caption;
    void *result;
    bool must_stop;
    quint64 curPos;
    virtual void run();
    virtual void processLine(QString &line) = 0;
    virtual void finishProcessing();
signals:
    void resultReady(void *result);
    void progress(void *result);
private slots:
    void addToList();
    void removeFromList();
    void stop_thread();
};

#endif // LOADERTHREAD_H
