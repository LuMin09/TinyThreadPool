#ifndef TASKQUEUE_H
#define TASKQUEUE_H
#include <iostream>
#include <queue>
#include <pthread.h>
#include "task.h"

class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();
public:
    inline size_t getCurrentTaskCount()
    {
        return m_taskQueue.size();
    }
    /*任务添加函数，生产者*/
    void addTask(Task::Ptr t_TaskPtr);
    Task::Ptr getTask();
private:
    std::queue<Task::Ptr> m_taskQueue;
    pthread_mutex_t m_mutex;
};
#endif