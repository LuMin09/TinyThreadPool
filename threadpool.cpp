#include "threadpool.h"
#include <string.h>
#include <unistd.h>
#define MANAGER_SLEEP_S 3
#define ADD_COUNT_THREAD 2
#define DEL_COUNT_THREAD 2
#define THRENDPOOL_MIN_COUNT 3
#define THREADPOOL_MAX_COUNT 10
ThreadPool::ThreadPool(int min, int max)
{
    //创建任务队列结构
    m_taskQ = new TaskQueue();
    do
    {
        //创建任务线程数组空间，大小为max
        m_taskThreads = new pthread_t[max];
        if(m_taskThreads == nullptr)
        {
            cout << "m_taskThreads new error"<<endl;
            break;
        }
        memset(m_taskThreads, 0, max * sizeof(pthread_t));
        minNum = min;
        maxNum = max;
        busyNum = 0;
        exitNum = 0;
        liveNum = min;

        //初始化线程同步结构
        if (pthread_mutex_init(&m_mutexPool, nullptr) != 0 ||
            pthread_cond_init(&m_notEmptyCond, nullptr) != 0)
        {
            cout << "mutex or condition init fail..." << endl;
            break;
        }

        //创建线程(管理线程与任务线程)
        pthread_create(&m_managerThread, nullptr, Manager, this);
        for (int i = 0; i < minNum; ++i)
        {
            pthread_create(&m_taskThreads[i], nullptr, Worker, this);
        }
        return;
    } while (0);
    //错误析构
    if(m_taskQ) delete m_taskQ;
    if(m_taskThreads) delete [] m_taskThreads;
}
ThreadPool::~ThreadPool()
{
    isShutdown = true;
    //阻塞等待回收管理线程
    pthread_join(m_managerThread,nullptr);
    for(int i = 0;i < liveNum; ++i)
    {
        //唤醒任务线程，由于isShutdown被激活，执行pthread_exit操作
        pthread_cond_signal(&m_notEmptyCond);
    }
    //释放堆内存
    if(m_taskQ)
    {
        delete m_taskQ;
        m_taskQ = nullptr;
    }
    if(m_taskThreads)
    {
        delete m_taskThreads;
        m_taskThreads = nullptr;
    }
    //释放线程同步结构体
    pthread_mutex_destroy(&m_mutexPool);
    pthread_cond_destroy(&m_notEmptyCond);
}
void *ThreadPool::Manager(void *arg)
{
   //ThreadPool *pool = (ThreadPool *)arg;
   ThreadPool *pool = static_cast<ThreadPool *>(arg);
    while (!pool->isShutdown)
    {
        //每隔三秒检测一次
        sleep(MANAGER_SLEEP_S);
        pthread_mutex_lock(&pool->m_mutexPool);
        int queueSize = pool->m_taskQ->getCurrentTaskCount();//获得任务列表任务数
        int liveNum = pool->liveNum;
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->m_mutexPool);

        //1.添加线程 以下逻辑比较简单，如果有更好的添加线程逻辑可以自行定义
        //什么情况加线程：任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if( queueSize > liveNum && liveNum < pool->maxNum)
        {
        int counter = 0;
        //加线程是从0-maxthreadCount选取0的线程进行创建线程；一次创建最多ADD_COUNT_THREAD个；存活的线程必须小于最大线程数
        pthread_mutex_lock(&pool->m_mutexPool);
        for(int i = 0; i < pool->maxNum && counter < ADD_COUNT_THREAD
            && pool->liveNum < pool->maxNum; ++i)
            {
                //判断为0的线程ID为未使用的线程，所以在线程自行销毁后也要将线程ID置为0
                if(pool->m_taskThreads[i] == 0)
                {
                    pthread_create(&pool->m_taskThreads[i],nullptr,Worker,pool);
                    counter++;
                    pool->liveNum++;
                }
            }
        }
        pthread_mutex_unlock(&pool->m_mutexPool);

        //2.销毁线程 以下逻辑比较简单，如果有更好的销毁线程逻辑可以自行定义
        //忙的线程*2 < 存活的线程数 && 存活的线程数 > 最小线程数
        if(busyNum *2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->m_mutexPool);
            pool->exitNum = DEL_COUNT_THREAD;
            pthread_mutex_unlock(&pool->m_mutexPool);
            //让工作的线程自杀
            for(int i = 0 ; i < DEL_COUNT_THREAD; ++i)
            {
                //唤醒线程操作时唯一的，谁抢到了wait这个条件变量的互斥锁谁就向下执行；所以一次只有一个线程进行向下操作
                pthread_cond_signal(&pool->m_notEmptyCond);
            }
        }
    }
    return nullptr;
}
void *ThreadPool::Worker(void *arg)
{
    //ThreadPool *pool = (ThreadPool *)arg;
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    while(1)
    {
        pthread_mutex_lock(&pool->m_mutexPool);
        //在任务数为0时（这里判断条件改为queue.isempty()是否能提高效率）并且非关闭情况。挂起循环体（阻塞线程）
        while (pool->m_taskQ->getCurrentTaskCount() == 0 && !pool->isShutdown)
        {
            cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
            pthread_cond_wait(&pool->m_notEmptyCond,&pool->m_mutexPool);//wait会先解锁，再加锁
             // 判断是不是要销毁线程 空闲线程过多，让挂起线程唤醒去自杀
            if (pool->exitNum > 0)
            {
                pool->exitNum--;//不管蛮不满足自杀要求，下发命令都要下发两次。且两次结束后break
                if (pool->liveNum > pool->minNum)//若或者的线程大于最小线程数 才进行线程销毁
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->m_mutexPool);
                    pool->threadExit();
                }
            }
        }
        if(pool->isShutdown)
        {
            pthread_mutex_unlock(&pool->m_mutexPool);
            // pthread_exit(NULL);
            pool->threadExit();
            // break;
        }
        //从任务队列取一个任务
        Task::Ptr t_TaskPtr = pool->m_taskQ->getTask();//是否需要判断指针为空?时效?

        pool->busyNum++;
        pthread_mutex_unlock(&pool->m_mutexPool);

        //执行任务
        t_TaskPtr->run();

        pthread_mutex_lock(&pool->m_mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->m_mutexPool);
    }
    return nullptr;   
}
void ThreadPool::threadExit()
{
    pthread_t self = pthread_self();
    //找到要销毁的当前线程执行pthread_exet并置为0
    for (int i = 0; i < maxNum; ++i)
    {
        if (self == m_taskThreads[i])
        {
            m_taskThreads[i] = 0;
            cout << "thread exit,the tid is " << to_string(self) << "..." << endl;
            break;
        }
    }
    pthread_exit(nullptr);
}
void ThreadPool::addTask(Task::Ptr t_TaskPtr)
{
    if(isShutdown)
    {
        return;
    }
    //添加任务
    m_taskQ->addTask(t_TaskPtr);
    //唤醒任务线程处理，存活的任务线程抢占条件变量，谁先抢到谁执行，若存活线程数与任务数满足某种条件添加线程执行
    pthread_cond_signal(&m_notEmptyCond);
}
int ThreadPool::getBusyNum()
{
    pthread_mutex_lock(&m_mutexPool);
    int busy = busyNum;
    pthread_mutex_unlock(&m_mutexPool);
    return busy;
}
int ThreadPool::getAliveNum()
{
    pthread_mutex_lock(&m_mutexPool);
    int Alive = liveNum;
    pthread_mutex_unlock(&m_mutexPool);
    return Alive;
}
ThreadPool &ThreadPool::Instance()
{
    static shared_ptr<ThreadPool>  s_instance(new ThreadPool(THRENDPOOL_MIN_COUNT,THREADPOOL_MAX_COUNT));    //c++11保证了多线程安全，程序退出时，释放资源
    static ThreadPool &s_insteanc_ref = *s_instance;
    return s_insteanc_ref;
}