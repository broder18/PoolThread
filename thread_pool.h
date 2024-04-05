#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <unordered_set>
#include "common.h"

class ThreadPool
{
public:
    explicit ThreadPool(int numThreads = 1);
    ~ThreadPool();

    template<typename Func, typename... Args>
    void addTask(Func&& func, Args&&... args)
    {
        long long idx = _last_idx++;
        std::lock_guard<std::mutex> lock(_mutex);
        std::function<void()> task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        _tasks.push_back(Task{task, idx});
        _cv.notify_one();
    }

    template<typename Func, typename... Args>
    bool removeTask(Func&& func, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::function<void()> task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        auto it = std::find_if(_tasks.begin(), _tasks.end(), [&task](const std::function<void()>& t) {
            return t.target_type() == task.target_type();
        });
        if (it != _tasks.end()) {
            _tasks.erase(it);
            return true;
        } else {
            return false;
        }
    }

    bool removeTask(const int id)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto isTargetValue = [&](const Task& task){
            return task.idx == id;
        };

       auto iterator = std::find_if(_tasks.begin(), _tasks.end(), isTargetValue);

       if (iterator != _tasks.end()) {
           _tasks.erase(iterator);
           return true;
       }

       return false;
    }

    void start();
    void stop();
    void wait(const long long task_id);
    void wait_all();
    void checkCalculated(const int task_id);
    void setNumThreads(const int num_threads);




private:
   std::vector<std::thread> _threads; //запускаемые потоки
   //std::vector<std::function<void()>> _tasks;
   std::vector<Task> _tasks; // принимаемые функци вместе с их айдишником

   std::unordered_set<long long> _completed_tasks_id;
   std::mutex _mutex; // синхронизация доступа к очереди задач
   std::mutex _completed_id_mutex;
   std::condition_variable _cv; //условная переменная для ожидания доступных задач
   std::condition_variable _completed_id_cv;
   std::atomic<bool> _running;
   std::atomic<long long> _last_idx;
   int _num_threads;

   void run();
};

#endif // THREADPOOL_H
