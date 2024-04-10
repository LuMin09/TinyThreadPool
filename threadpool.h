#ifndef THREDPOOL_H
#define THREDPOOL_H
#include <iostream>
#include <list>
#include <queue>
#include <sys/wait.h>
#include <pthread.h>
#include "task.h"
#include "taskqueue.h"
using namespace std;

class ThreadPool
{
    //单例必须私有化的成员
private:
    /*最大的线程数和最小的线程数*/
    ThreadPool(int min,int max);
    /*拷贝构造函数不实现，防止拷贝产生多个实例*/
    ThreadPool(const ThreadPool &);
    /*复制构造函数不实现，防止赋值产生多个实例*/
    ThreadPool operator = (const ThreadPool &);
public:
    /*单例，提供全局访问点,线程最大最小值请根据宏进行修改*/
    static ThreadPool &Instance();
    ~ThreadPool();
    /*对外提供添加任务体函数，参数为任务体基类智能指针，可自己继承实现*/
    void addTask(Task::Ptr t_TaskPtr);
    /*获得当前忙碌线程个数*/
    int getBusyNum();
    /*获得当前存活线程个数*/
    int getAliveNum();
private:
    /*管理者操作*/
    static void *Manager(void *arg);
    /*任务执行操作，消费者*/
    static void *Worker(void *arg);
    /*单个线程退出*/
    void threadExit();

private:
    pthread_t m_managerThread;         //管理者线程
    pthread_t* m_taskThreads;          //任务线程队列
    pthread_mutex_t m_mutexPool;       //线程互斥锁
    pthread_cond_t m_notEmptyCond;     //唤醒非空条件变量
          
    TaskQueue* m_taskQ;                //任务队列
    int minNum;     //最小线程数
    int maxNum;     //最大线程数
    int busyNum;    //在工作线程数
    int exitNum;    //要退出的线程数
    int liveNum;    //存活的线程数

    bool isShutdown = false; //是否要被关闭
};
#endif