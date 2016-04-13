#ifndef COMMON
#define COMMON

#include <QReadWriteLock>

class QReadWriteLocker
{
    QReadWriteLock *lock;
public:
    QReadWriteLocker(QReadWriteLock *_lock, bool write=false): lock(_lock)
    {
        if ( write )
            lock->lockForWrite();
        else
            lock->lockForRead();
    }
    ~QReadWriteLocker()
    {
        lock->unlock();
    }
};

#endif // COMMON

