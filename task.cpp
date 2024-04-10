#include "task.h"
Task::Task()
{
    arg = nullptr;
}
Task::~Task()
{
    if(arg)
    {
        delete arg;
        arg = nullptr;
    }
}
void Task::SetParam(void *arg)
{
    arg = arg;
}