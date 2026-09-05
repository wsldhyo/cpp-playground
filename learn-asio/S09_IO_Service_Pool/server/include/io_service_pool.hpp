#ifndef IO_SERVICE_POOL_HPP
#define IO_SERVICE_POOL_HPP
#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>

#include "../../common/singleton.hpp"

class IOServicePool : public Singleton<IOServicePool>{
public:
    friend class Singleton<IOServicePool>;
    // 之前的io_context.run()之所以被阻塞，是因为服务器中acceptor注册了一个监听对端连接的事件
    // 而acceptor绑定了io_context, 这就相当于在io_context里面绑定了监听读事件， 让io_context.run
    // 时，去监听读事件并阻塞，否则run函数是会立即退出的

    // work已经被弃置，使用executor_work_gaurd代替
    //using Work_t = asio::io_context::work; 
    using Work_t = asio::executor_work_guard<asio::io_context::executor_type>;
    using Work_Uptr_t = std::unique_ptr<Work_t>;
    ~IOServicePool();
    
    // 使用round-robin方式取出一个io_context
    asio::io_context& get_io_context();
    void stop();    
    // 禁止拷贝
    IOServicePool(IOServicePool const&) = delete;
    IOServicePool& operator=(IOServicePool const&) = delete; 
private:
    IOServicePool(std::size_t size = std::thread::hardware_concurrency());

    std::vector<asio::io_context> io_contexts_;
    std::vector<Work_Uptr_t> Works_;
    std::vector<std::thread> threads_;
    std::size_t next_io_context_index_;
};
#endif