#ifndef IO_CONTEXT_POOL_HPP
#define IO_CONTEXT_POOL_HPP
#include <cstddef>
#include <thread>
#include <memory>
#include <asio/io_context.hpp>
#include <asio/executor_work_guard.hpp>
#include <vector>
class IOContextPool {
public:
  using io_context_t = asio::io_context;
  using wort_t = asio::executor_work_guard<io_context_t::executor_type>;
  using work_uptr_t = std::unique_ptr<wort_t>; 

  ~IOContextPool();
  IOContextPool(IOContextPool const&)=delete;
  IOContextPool& operator=(IOContextPool const&)=delete;
  io_context_t& get_io_context();
  void stop();
  static IOContextPool& get_instance();
private:
    IOContextPool(std::size_t size = std::thread::hardware_concurrency());

    std::vector<io_context_t> io_contexts_;
    std::vector<work_uptr_t> works_;
    std::vector<std::thread> threads_;
    std::size_t io_context_index_;

};

#endif