
#include <asio.hpp>
#include <asio/detail/socket_ops.hpp>
#include <asio/error.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>

#include "../proto_msg/msg.pb.h"
using asio::ip::tcp;

constexpr std::size_t MAX_MSG_LEN{2 * 1024};
constexpr std::size_t MSG_HEAD_LEN{2};
int main(int argc, char *argv[]) {
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
    MsgData msgdata;
    msgdata.set_id(1001);
    msgdata.set_data("Hello World");
    std::string send_msg;
    msgdata.SerializeToString(&send_msg);
    short send_msg_len = send_msg.length();
    char send_data[MAX_MSG_LEN];
    // 写入的头部信息是使用网路字节序表示的信息
    auto net_send_msg_len =
        asio::detail::socket_ops::host_to_network_short(send_msg_len);
    memcpy(send_data, &net_send_msg_len, MSG_HEAD_LEN);
    memcpy(send_data + MSG_HEAD_LEN, send_msg.c_str(), send_msg_len);

    // 这里写入长度要用本地字节序表示的长度send_msg_len，而不是网络字节序表示的长度
    asio::write(sock, asio::buffer(send_data, send_msg_len + MSG_HEAD_LEN));

    char reply_head[MSG_HEAD_LEN];
    auto reply_len{asio::read(sock, asio::buffer(reply_head, MSG_HEAD_LEN))};

    short recv_msg_len = 0;
    memcpy(&recv_msg_len, reply_head, MSG_HEAD_LEN);
    // 先将头部信息转为本地字节序，获取实际长度
    recv_msg_len =
        asio::detail::socket_ops::network_to_host_short(recv_msg_len);
    char reply_buf[MAX_MSG_LEN];
    asio::read(sock, asio::buffer(reply_buf, recv_msg_len));
    MsgData recv_msg;
    recv_msg.ParseFromString(reply_buf);
    std::cout << "Reply: Msg id is " << recv_msg.id() << " msg is " << recv_msg.data();
    getchar();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}
