#include "../include/io_context_pool.hpp"
#include "../include/server.hpp"
#include <asio/signal_set.hpp>
#include <csignal>
#include <exception>
#include <filesystem>
#include <iostream>
int main(int argc, char *argv[]) {
  try {
    // 拼接本地路由
    std::filesystem::path path = std::filesystem::current_path() / "res";
    // 使用 std::cout 输出拼接后的路径
    std::cout << "Path: " << path.string() << '\n';
    std::cout << "Usage: http_server <127.0.0.1> <8080> " << path.string()
              << "\n";
    // Initialise the server.
    http::server::server s("127.0.0.1", "8080", path.string());
    // Run the server until stopped.
    s.run();
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << "\n";
  }
  return 0;
}