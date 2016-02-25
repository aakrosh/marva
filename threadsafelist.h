#ifndef THREDSAFELIST_H
#define THREDSAFELIST_H

#include <QMutex>
#include <QList>

template <class T>
class ThreadSafeList : public QList<T>
{
    QMutex	m_mutex;
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
    ThreadSafeListLocker(ThreadSafeList<T> *list): tslist(list)
    {
        tslist->m_mutex.lock();
    }
    ~ThreadSafeListLocker()
    {
        tslist->m_mutex.unlock();
    }
};

//=========================================================================
template <class T>
void ThreadSafeList<T>::Add(T t)
{
    m_mutex.lock();
    this->append(t);
    m_mutex.unlock();
}

//=========================================================================
template <class T>
void ThreadSafeList<T>::Delete(T t)
{
    m_mutex.lock();
    this->removeOne(t);
    m_mutex.unlock();
}

#endif // THREDSAFELIST_H
