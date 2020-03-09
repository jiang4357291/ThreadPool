#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <future>
#include <memory>
#include <condition_variable>


class ThreadPool
{
    using Task = std::function<void()>;
public:
    ThreadPool(size_t num);
    ~ThreadPool();
    void entry();
    void shutdown();
    template <class Function,class... Args>
    // add_task相当于是对一个通用函数包装器的改造，不同点在于通用包装器的返回值可以return，而这里添加的task还未被执行
    // 需要等到将来工作线程来取，因此其返回类型的实现就需要用到std::future和std::packaged_task
    auto add_task(Function &&f, Args&&... args) -> std::future<decltype(f(std::forward<Args>(args)...))> 
    {
        using return_type = decltype(f(std::forward<Args>(args)...));
        // 通过std::bind()构造出一个void()可调用对象
        auto fun = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
        // 通过packaged_task获取一个异步操作的返回值，返回值也就是fun()也就是f(std::forward<Args>(args)...)的返回值
        auto task = std::make_shared<std::packaged_task<return_type()>>(fun);
        auto res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(task_mutex);
            tasks.emplace([task]{(*task)();});
        }

        cond.notify_one();
        return res;
    }


    /* 问题版本
    auto add_task(Function &&f, Args&&... args) -> std::future<decltype(f(std::forward<Args>(args)...))> 
    {
        using return_type = decltype(f(std::forward<Args>(args)...));
        auto fun = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
        auto res = std::packaged_task<return_type()>(fun).get_future();

        {
            std::unique_lock<std::mutex> lock(task_mutex);
            tasks.emplace(std::move(fun));
        }

        cond.notify_one();
        return res;
    }
    */
private:
    std::vector<std::thread> threads;
    std::condition_variable cond;
    bool stop;
    std::queue<Task> tasks;// task排队应该先进先出，所以用队列
    std::mutex task_mutex;
};

#endif