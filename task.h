#ifndef TASK_H
#define TASK_H
#include <iostream>
#include <memory>
class Task
{
public:
    typedef std::shared_ptr<Task> Ptr;
    Task();
    /*析构void*指针存在警告，请用模板去除或弃用该结构，以在派生类添加参数方式为解决方案*/
    virtual ~Task();
    /*请确保传入的值为堆数据*/
    void SetParam(void *arg);
public:
    virtual void run() = 0;
private:
    void *arg;
    
};
#endif