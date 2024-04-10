#include "threadpool.h"
#include "task.h"
#include <unistd.h>
#include <functional>
#include <list>
class TaskDemo : public Task
{
public:
    TaskDemo();
    ~TaskDemo();
    void run();
    int i;
};

TaskDemo::TaskDemo()
{
}

TaskDemo::~TaskDemo()
{
}
void TaskDemo::run()
{
    cout << "thread tid is " << pthread_self() <<", num is " << 100 + i << endl;
    sleep(1);
}
int main()
{
    ThreadPool::Instance();//初始化
    for(int i = 0; i < 1000; i++)
    {
        std::shared_ptr<TaskDemo> ptr = std::make_shared<TaskDemo>();
        ptr->i = i;
        ThreadPool::Instance().addTask(ptr);//添加任务 存在隐式类型转换
    }
    std::shared_ptr<TaskDemo> ptr1 = std::make_shared<TaskDemo>();
    ptr1->i = 520;
    ThreadPool::Instance().addTask(ptr1);

    std::shared_ptr<TaskDemo> ptr2 = std::make_shared<TaskDemo>();
    ptr2->i = 1314;
    ThreadPool::Instance().addTask(ptr2);
    while(1)
    {
        std::cout << "------------Alive Num is :" << ThreadPool::Instance().getAliveNum() << endl;
        std::cout << "++++++++++++Busy Num is :" << ThreadPool::Instance().getBusyNum() << endl;
        usleep(200*1000);
    }
    getchar();//阻塞主线程退出
    // typedef std::function<void *() > cb;
    // std::list<cb> cdList;
    // cdList.push_back([&]()->void *{
    //     int i = 0;
    //     cout << i << endl;
    // });

    // cdList.back()();
 
    return 0;
}