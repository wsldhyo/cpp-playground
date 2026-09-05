
#include <cstddef>
#include <cstring>

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <asio.hpp>
#include <asio/detail/socket_ops.hpp>
#include <asio/error.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <json/forwards.h>
#include <json/reader.h>
#include <json/value.h>

#include "../../common/constant.hpp"
#include "../../common/msg_node.hpp"

using asio::ip::tcp;

constexpr std::size_t MAX_MSG_LEN{2 * 1024};
constexpr std::size_t MSG_HEAD_LEN{2};
int main(int argc, char *argv[]) {

  std::vector<std::thread> threads;

  auto start_time = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100; ++i) {
    std::cout << "thread " << i << "start"<<std::endl;
    threads.emplace_back([i]() -> int {
      try {
        asio::io_context ioc;
        // 对端地址，即服务器地址
        tcp::endpoint remote_ep(asio::ip::make_address("127.0.0.1"), 10086);
        asio::error_code ec = asio::error::host_not_found;
        tcp::socket sock(ioc);
        // 向服务器发起连接请求
        sock.connect(remote_ep, ec);
        if (ec.value() != 0) {
          std::cout << "connect failed error code = " << ec.value()
                    << " Message: " << ec.message() << std::endl;
          return ec.value();
        }
        int j = 0;

        Json::Value root;
        root["id"] = 1001;
        root["data"] = "Hello World";
        auto send_msg = root.toStyledString();
        auto send_node = std::make_shared<SendNode>(send_msg.data(),
                                                    send_msg.length(), 1001);
        auto recv_head_node =
            std::make_shared<MsgNode>(HEAD_ID_LENGTH + HEAD_DATA_LENGTH);
        std::size_t reply_len;
        short recv_msg_len = 0;
        short recv_msg_id = 0;
        Json::Reader reader;
        std::string recv_msg;
        while (j < 500) {
          // 这里写入长度要用本地字节序表示的长度send_msg_len，而不是网络字节序表示的长度
          asio::write(sock,
                      asio::buffer(send_node->data, send_node->total_len));
          recv_head_node->clear();
          reply_len = asio::read(sock, asio::buffer(recv_head_node->data,
                                                    recv_head_node->total_len));

          memcpy(&recv_msg_id, recv_head_node->data, HEAD_ID_LENGTH);
          memcpy(&recv_msg_len, recv_head_node->data + HEAD_ID_LENGTH,
                 HEAD_DATA_LENGTH);
          // 先将头部信息转为本地字节序，获取实际长度
          recv_msg_len =
              asio::detail::socket_ops::network_to_host_short(recv_msg_len);
          recv_msg_id =
              asio::detail::socket_ops::network_to_host_short(recv_msg_id);
          auto recv_msg_node =
              std::make_shared<RecvNode>(recv_msg_len, recv_msg_id);
          asio::read(sock, asio::buffer(recv_msg_node->data, recv_msg_len));

          recv_msg.assign(recv_msg_node->data, recv_msg_len);
          reader.parse(recv_msg, root);
        std::cout << "Reply id is " << recv_msg_id << " len is "
                  << recv_msg_len << " data is " << root["data"].asString()
                  << std::endl;
          ++j;
        }
      } catch (std::exception const &e) {
        std::cerr << "Exception code is " << e.what() << std::endl;
      }
      return 0;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  for (auto &thread : threads) {
    thread.join();
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::cout << "Cost time:"
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                     start_time)
                   .count();
  return 0;
}
