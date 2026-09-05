#include "../include/io_context_pool.hpp"
#include <asio/executor_work_guard.hpp>
#include <cstddef>
#include <memory>

    IOContextPool::IOContextPool(std::size_t size/*std::thread::hardware_concurrency()*/):
        io_contexts_(size),
        works_(size),
        io_context_index_(0)
    {
        for(std::size_t i = 0; i < size; ++i){
             works_[i] =std::move(std::make_unique<wort_t>(asio::make_work_guard(io_contexts_[i])));
        }
    
        for(std::size_t i = 0; i < io_contexts_.size(); ++i)
        {
            threads_.emplace_back([this, i]{
                io_contexts_[i].run();
            });
        }
    }

  IOContextPool::~IOContextPool(){

  }

  IOContextPool::io_context_t& IOContextPool::get_io_context(){
    auto& io_context = io_contexts_[io_context_index_++];
    if (io_context_index_ == io_contexts_.size()) {
        io_context_index_ = 0;
    }
    return io_context;
  }
  
  void IOContextPool::stop(){
    for(auto& work: works_)
    {
        work.reset();
    }
    for (auto& thread : threads_) {
        thread.join();
    }
  }

  IOContextPool& IOContextPool::get_instance(){
    static IOContextPool instance(1);
    return instance;

  }