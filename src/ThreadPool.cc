#include <iostream>
#include "ThreadPool.h"

using namespace std;

ThreadPool::ThreadPool(size_t num):stop(false)
{
    for(int i=0;i<num;++i)
        threads.emplace_back(&ThreadPool::entry,this);
}

void ThreadPool::entry()
{
    while(1)
    {
        unique_lock<mutex> lock(task_mutex);
        // 当任务队列为空且线程池正常状态时才会阻塞，被唤醒可能是由于有新任务到来或者是线程池准备停止
        cond.wait(lock,[this]{return this->stop||!this->tasks.empty();});
        // 如果是线程池停止，则退出子线程
        if(stop)
            return;

        // 否则就从任务队列中取最靠前的一个任务进行执行，并将任务出队
        Task cur_task = std::move(tasks.front());
        tasks.pop();
        lock.unlock();// task取出后就可以解锁
        cur_task();
    }
}

ThreadPool::~ThreadPool()
{
    cout<<"delete pool!"<<endl;
    if(!stop)
        shutdown();
}

void ThreadPool::shutdown()
{
    // 修改stop为true并通知所有线程池中的线程
    {
        unique_lock<mutex> lock(task_mutex);
        stop = true;
    }
    cond.notify_all();
    // 同时等待子线程执行完成
    for(auto &th: threads)
        th.join();
}
