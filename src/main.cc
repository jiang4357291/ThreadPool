#include <iostream>
#include <chrono>
#include "ThreadPool.h"

using namespace std;

int main()
{

    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(// std::future不能复制，所以这里必须用emplace_back原地构造
            pool.add_task([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    // .get()会阻塞直到获取到结果，如果shutdown放到get之前则get会一直阻塞
    pool.shutdown();
    std::cout << std::endl;
    
    return 0;
}
