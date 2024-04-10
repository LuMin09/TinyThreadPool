#include "taskqueue.h"
TaskQueue::TaskQueue()
{
    pthread_mutex_init(&m_mutex,nullptr);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}
void TaskQueue::addTask(Task::Ptr t_TaskPtr)
{
    pthread_mutex_lock(&m_mutex);
    m_taskQueue.push(t_TaskPtr);
    pthread_mutex_unlock(&m_mutex);
}
Task::Ptr TaskQueue::getTask()
{
    Task::Ptr t_lpTask = nullptr;
    pthread_mutex_lock(&m_mutex);
    if(m_taskQueue.empty())
    {
        pthread_mutex_unlock(&m_mutex);
        return t_lpTask;
    }
    t_lpTask = m_taskQueue.front();
    m_taskQueue.pop();
    pthread_mutex_unlock(&m_mutex);
    return  t_lpTask;
}