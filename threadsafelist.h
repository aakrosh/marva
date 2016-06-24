#ifndef THREDSAFELIST_H
#define THREDSAFELIST_H

#include "common.h"

#include <QReadWriteLock>
#include <QList>

template <class T>
class ThreadSafeList : public QList<T>
{
    QReadWriteLock	lock;
public:
    virtual void Add(T t);
    virtual void Delete(T t);

    template<class T1>
    friend class ThreadSafeListLocker;
};

template <class T>
class ThreadSafeListLocker
{
    ThreadSafeList<T> *tslist;
public:
    ThreadSafeListLocker(ThreadSafeList<T> *list, bool write = false): tslist(list)
    {
        if ( write )
            tslist->lock.lockForWrite();
        else
            tslist->lock.lockForRead();
    }
    ~ThreadSafeListLocker()
    {
        tslist->lock.unlock();
    }
};

//=========================================================================
template <class T>
void ThreadSafeList<T>::Add(T t)
{
    QReadWriteLocker locker(&lock, true);
    this->append(t);
}

//=========================================================================
template <class T>
void ThreadSafeList<T>::Delete(T t)
{
    QReadWriteLocker locker(&lock, true);
    this->removeOne(t);
}

#endif // THREDSAFELIST_H
