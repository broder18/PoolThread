#include "thread_pool.h"
#include <QDebug>

ThreadPool::ThreadPool(int numThreads) : _running(false), _last_idx(), _num_threads(numThreads)
{
    _threads.reserve(numThreads);
}

ThreadPool::~ThreadPool()
{
    stop();
}


void ThreadPool::start()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_running)
        return;

    _running = true;

    for(auto ind = 0; ind < _num_threads; ind++)
    {
        _threads.push_back(std::thread(&ThreadPool::run, this));
    }

}

void ThreadPool::stop()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(!_running)
        return;

    _running = false;
    _cv.notify_all();
    lock.unlock();

    for(auto ind = 0; ind < _threads.size(); ++ind)
    {
        if(_threads[ind].joinable())
            _threads[ind].join();
    }

    _threads.clear();
    _tasks.clear();
}

void ThreadPool::setNumThreads(const int num_threads)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _num_threads = num_threads;
    _threads.resize(num_threads);
}


void ThreadPool::wait(const long long task_id)
{
    std::unique_lock<std::mutex> lock(_completed_id_mutex);

    //ждём notify с функции run, сработает после завершения задачи
    _completed_id_cv.wait(lock, [this, task_id]()->bool
    {
        return _completed_tasks_id.find(task_id) != _completed_tasks_id.end();
    });
}

void ThreadPool::wait_all()
{
    std::unique_lock<std::mutex> lock (_mutex);

    _completed_id_cv.wait(lock, [this]()->bool
    {
        std::lock_guard<std::mutex> task_lock(_completed_id_mutex);
        return _tasks.empty() && _last_idx == _completed_tasks_id.size();
    });
}

void ThreadPool::run()
{
    while(true)
    {

        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [this]()
        {
            return !_tasks.empty() || _running;
        });

        if(_tasks.empty())
            return;

        Task task = _tasks.front();
        _tasks.erase(_tasks.begin());

        task.func();
        std::lock_guard<std::mutex> lock_g(_completed_id_mutex); //блочим переменную с выполненными id
        _completed_tasks_id.insert(task.idx);
        _completed_id_cv.notify_all();
    }
}
