#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <cstddef>

#include <memory>
#include <thread>
#include <vector>

#include <asio/io_context.hpp>
#include <asio/executor_work_guard.hpp>

#include "../../common/singleton.hpp"

class AsioThreadPool : public Singleton<AsioThreadPool>
{
public:
    friend class Singleton<AsioThreadPool>;
    using Work_t = asio::executor_work_guard<asio::io_context::executor_type>;
    using Work_Uptr_t = std::unique_ptr<Work_t>;

public:  
    ~AsioThreadPool();
    AsioThreadPool(AsioThreadPool const&)=delete;
    AsioThreadPool& operator=(AsioThreadPool const&)=delete;
    asio::io_context& get_io_context();
    void stop();
    

private:
    AsioThreadPool(std::size_t thread_num = std::thread::hardware_concurrency());
    asio::io_context io_context_;
    Work_Uptr_t work_;
    std::vector<std::thread> threads_;
};

#endif