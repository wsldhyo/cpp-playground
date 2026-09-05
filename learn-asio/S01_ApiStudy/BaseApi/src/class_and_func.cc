#include <asio/buffer.hpp>
#include <asio/detail/string_view.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address_v4.hpp>
#include <asio/ip/address_v6.hpp>
#include <asio/ip/basic_endpoint.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

#include <asio/error_code.hpp>
#include <asio/ip/address.hpp>
#include <system_error>
#include <vector>

#include "../include/class_and_func.hpp"
inline int client_end_point() {
  // 需要知道对端（服务器）的ip和端口号
  std::string raw_ip_address = "127.4.8.1";
  port_t port_num = 3333;

  asio::error_code ec;
  // make_address替代了boost版中的from_string函数
  asio::ip::address ip_address = asio::ip::make_address(raw_ip_address);
  if (ec.value() != 0) {
    std::cout << "Failed to parsed the IP address. Error Code: " << ec.value()
              << " Message: " << ec.message() << std::endl;
    return ec.value();
  }
  asio::ip::tcp::endpoint ep(ip_address, port_num);
  return 0;
}

inline int server_end_point() {
  port_t port_num = 3333; // 对端端口可以是任意的吗？？
  // 对端可以是任意ip地址来访问服务器
  asio::ip::address ip_address = asio::ip::address_v6::any();
  asio::ip::tcp::endpoint ep(ip_address, port_num);
  return 0;
}

inline int create_tcp_socket() {
  // asio所有服务都是通过io_context对象，这是asio的核心类
  asio::io_context ioc;
  asio::ip::tcp protocol = asio::ip::tcp::v4();
  asio::ip::tcp::socket sock(ioc);
  // 判断socket是否创建成功
  asio::error_code ec;
  sock.open(protocol, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to open the socket! Error code:" << ec.value()
              << " Message: " << ec.message() << std::endl;
    return ec.value();
  }
  return 0;

  // 新版asio
  // 可以更简单的打开sock，不需要后面的open检测了，内部在连接通信时会自行判断G
}

inline int create_acceptor_socket() {
  asio::io_context ioc_server;
  /*
   老版本需要向上面create_tcp_socket那样，先打开acceptor判断是否成功
  asio::ip::tcp::acceptor acceptor(ios);
  asio::ip::tcp protocol = asio::ip::tcp::v4();
  asio::error_code ec;
  acceptor.open(protocol, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to open the socket! Error code:" << ec.value()
              << " Message: " << ec.message() << std::endl;
    return ec.value();
  }
  */

  // 新版本只需要io_context和端点即可
  // 创建时即绑定端点，也可以向bind_acceptor_socket()里先创建acceptor再绑定端点
  asio::ip::tcp::acceptor a(ioc_server,
                            asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
}

inline int bind_acceptor_socket() {
  port_t port_num = 3333;
  asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
  asio::io_context ios;
  asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
  std::error_code ec;
  acceptor.bind(ep, ec);
  if (ec.value() != 0) {
    std::cout << "Failed to bind the acceptor socket "
              << "Error code = " << ec.value() << "Message: " << ec.message()
              << std::endl;
    return ec.value();
  }
  return 0;
  // 也可以向create_acceptor_socket()里那样，创建即绑定端点
}

inline int connect_to_end() {
  std::string raw_ip_address = "192.168.1.124";
  port_t port_num = 3333;
  try {
    asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),
                               port_num);
    asio::io_context ios;
    asio::ip::tcp::socket sock(ios, ep.protocol());
    sock.connect(ep);
  } catch (std::error_code e) {
    std::cout << "Error occured! Error Code = " << e.value()
              << "Message: " << e.message() << std::endl;
    return e.value();
  }
  return 0;
};

inline int dns_connect_to_end() {
  std::string host = "llfc.club";
  std::string port_num = "3333";
  asio::io_context ios;
  // 旧版Asio
  // asio::ip::tcp::resolver resolver(ios);
  // asio::ip::tcp::resolver::query resolver_query(host,
  // port_num,asio::ip::resolver::query::numeric_service);
  // try {
  // 域名会对应多个ip
  // asio::ip::tcp::resolver::iterator  it = resolver::resolve(resolver_query);
  // asio::ip::tcp::socket sock(ios);
  // asio::connect(sock, it);
  // } catch (std::system_error&  e) {
  //   std::cout << "Error occured! Error Code = " << e.code().value() <<
  //     "Message: " << e.what() << std::endl;
  //     return e.code().value();
  // }

  // 新版Asio
  try {
    // 创建 resolver 对象
    asio::ip::tcp::resolver resolver(ios);

    // 解析主机和端口号，直接传递字符串
    auto resolve_res = resolver.resolve(host, port_num);
    // 域名可以对应多个ip
    auto endpoint = *resolve_res.begin();
    // 创建 socket
    asio::ip::tcp::socket sock(ios);

    // 连接到解析到的第一个端点
    sock.connect(endpoint);

    std::cout << "Connected to " << host << " on port " << port_num
              << std::endl;
  } catch (std::system_error &e) {
    std::cout << "Error occurred! Error Code = " << e.code().value()
              << ", Message: " << e.code().message() << std::endl;
    return e.code().value();
  }

  return 0;
}

inline int accept_new_connection() {
  int const BACKLOG_SIZE = 30; // 监听队列的大小，实际比30大
  port_t port_num = 3333;
  asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
  asio::io_context ios;
  try {
    asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
    acceptor.bind(ep);
    acceptor.listen(BACKLOG_SIZE); // 监听
    asio::ip::tcp::socket sock(ios);
    acceptor.accept(sock);
  } catch (std::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code().value()
              << "Message: " << e.code().message() << std::endl;

    return e.code().value();
  }
  return 0;
}

inline void use_const_buffer() {
  std::string str = "Hello World";
  asio::const_buffer asio_buf(str.c_str(), str.length());
  std::vector<asio::const_buffer> buffers_seqence;
  buffers_seqence.push_back(asio_buf);
}

inline void use_buffer_str() {
  // 旧版Asio
  // asio::const_buffers_1 output_buf = asio::buffer("Hello World");

  // 新Asio中，const_buffers_1被移除
  asio::const_buffer buf = asio::buffer("Hello World");
}

inline void use_buffer_arr() {
  std::size_t const BUF_SIZE_BYTES = 20;
  // TODO 怎么转为std::make_unique
  std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);
  auto input_buf = asio::buffer(static_cast<void *>(buf.get()), BUF_SIZE_BYTES);
}

inline void write_to_socket(asio::ip::tcp::socket &sock) {
  std::string str = "Hello World";
  std::size_t total_bytes_written = 0;
  // 循环发送数据
  // 用户缓冲区可以根据OS开辟足够大的空间, 而TCP缓冲区有限，所以每次发送一点
  // write_some每次返回实际写入到TCP缓冲区的字节数
  while (total_bytes_written != str.length()) {
    auto buf =
        asio::buffer(static_cast<void *>(str.data() + total_bytes_written),
                     str.length() - total_bytes_written);
    total_bytes_written += sock.write_some(buf);

  }
}

inline int send_data_by_write_some() {
  std::string raw_ip_string = "192.168.3.11";
  port_t port_num = 3333;
  try {
    asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_string), port_num);
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc, ep.protocol());
    sock.connect(ep);
    // 一次发送所有数据，
    //write_to_socket(sock);
    std::string buf{"Hello World"};
    // 数据没发送完，则阻塞。
    // send返回值有三种情况，小于0系统错误，=0对方关闭连接，>0发送完毕并且返回值一定等于发送长度 
    auto send_length {sock.send(asio::buffer(buf.data(), buf.length()))};
    if(send_length < 0){
      return 0;
    }
  } catch (std::system_error &e) {
    std::cout << "Error occured! Error Code = " << e.code().value()
              << "Message: " << e.code().message() << std::endl;
    return e.code().value();
  }
  return 0;
}

inline std::string read_from_socket(asio::ip::tcp::socket& sock){
  unsigned char const MESSAGE_SIZE {20};
  char buf[MESSAGE_SIZE]{0};
  std::size_t total_bytes_read{0};
  // read_some每次读取一定数量的内存，并返回实际读取长度
  // 每读满buf时，一直读取
  while (total_bytes_read != MESSAGE_SIZE) {
    total_bytes_read += sock.read_some(asio::buffer(buf +total_bytes_read, MESSAGE_SIZE - total_bytes_read));
  }


}

inline int write_data_by_write_some() {
  std::string raw_ip_string = "192.168.3.11";
  port_t port_num = 3333;
  try {
    asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_string), port_num);
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc, ep.protocol());
    sock.connect(ep);
    unsigned char const BUF_SIZE{7};
    char buffer_receive[BUF_SIZE];
    // 返回值类似socket::send
    // 也可以用asio::read(sock, buffer);来读取
    std::size_t bytes_receive_len{sock.receive(asio::buffer(buffer_receive, BUF_SIZE))};
    if(bytes_receive_len <= 0){
      std::cout << "receive failed" << std::endl;
      return 0;
    }
  } catch (std::system_error &e) {
    std::cout << "Error occured! Error Code = " << e.code().value()
              << "Message: " << e.code().message() << std::endl;
    return e.code().value();
  }
  return 0;
}