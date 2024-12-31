#include "tqueue.hpp"
#include <condition_variable>
#include <deque>
#include <chrono>
#include <mutex>
#include <optional>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <deque>
/*  写在前面
 *
 *  std::chrono::steady_clock::duration 代表一个时间段，类比与1个小时，20秒这种
 *  std::chrono::steady_clock::time_point 代表自开机以来的时间
 *
 */
template <class T, class Deque = std::deque<T>>
struct mt_queue
{
private:
    Deque m_queue;
    QMutex m_mutex;
    QWaitCondition m_cv_empty;
    QWaitCondition m_cv_full;
    std::size_t m_limit;

public:
    mt_queue() : m_limit(static_cast<std::size_t>(-1)) {}

    explicit mt_queue(std::size_t limit) : m_limit(limit) {}

    void push(T value)
    {
        QMutexLocker locker(&m_mutex); // 使用 QMutexLocker 自动管理锁
        while (m_queue.size() >= m_limit)
            m_cv_full.wait(&m_mutex);
        m_queue.push_front(std::move(value));
        m_cv_empty.wakeOne();
    }

    bool try_push(T value)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_limit)
            return false;
        m_queue.push_front(std::move(value));
        m_cv_empty.wakeOne();
        return true;
    }

    bool try_push_for(T value, std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_full.wait(&m_mutex, timeout.count());
        if (!success || m_queue.size() >= m_limit)
            return false;
        m_queue.push_front(std::move(value));
        m_cv_empty.wakeOne();
        return true;
    }

    bool try_push_until(T value, std::chrono::steady_clock::time_point timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_full.wait(&m_mutex, timeout.time_since_epoch().count());
        if (!success || m_queue.size() >= m_limit)
            return false;
        m_queue.push_front(std::move(value));
        m_cv_empty.wakeOne();
        return true;
    }

    T pop()
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.empty())
            m_cv_empty.wait(&m_mutex);
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> try_pop()
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> try_pop_for(std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_empty.wait(&m_mutex, timeout.count());
        if (!success || m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> try_pop_until(std::chrono::steady_clock::time_point timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_empty.wait(&m_mutex, timeout.time_since_epoch().count());
        if (!success || m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }
};

template <class T, class Deque = std::deque<T>>
class myThreadQueue
{
private:
    Deque m_queue;
    QMutex m_mutex;
    QWaitCondition m_cv_empty;
    QWaitCondition m_cv_full;
    std::size_t m_limit;

public:
    myThreadQueue() : m_limit(std::numeric_limits<std::size_t>::max()) {};

    explicit myThreadQueue(std::size_t limit) : m_limit(limit) {};

    void push(T value)
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.size() >= m_limit) // 队列满了，等待
        {
            m_cv_full.wait(&m_mutex);
        }
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
    }

    bool tryPush(T value)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_limit)
            return false;
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
        return true;
    }

    bool tryPushFor(T value, std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_full.wait(&m_mutex, timeout.count());
        if (!success)
            return false;
        m_queue.push_front(value);
        m_cv_empty.wakeOne();
        return true;
    }

    T pop()
    {
        QMutexLocker locker(&m_mutex);
        while (m_queue.empty())
        {
            m_cv_empty.wait(&m_mutex);
        }
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> tryPop()
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.empty())
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }

    std::optional<T> tryPopFor(std::chrono::steady_clock::duration timeout)
    {
        QMutexLocker locker(&m_mutex);
        bool success = m_cv_empty.wait(&m_mutex, timeout.count());
        if (!success)
            return std::nullopt;
        T value = std::move(m_queue.back());
        m_queue.pop_back();
        m_cv_full.wakeOne();
        return value;
    }
};
