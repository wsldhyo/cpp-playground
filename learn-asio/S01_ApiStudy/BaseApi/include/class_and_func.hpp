#ifndef CALSS_AND_FUNC_HPP
#define CALSS_AND_FUNC_HPP
//
// @brief 利用Asio完成网络通信中流程中各个独立部分的代码
//     片段，包括端点的创建、socket绑定端点、服务器监听
//     并接受客户端、客户端请求连接的相关代码
//


#include <asio/ip/tcp.hpp>
using port_t = unsigned short;
//
// @brief 创建客户端通信端点
//  Asio网络编程的第一步就是创建端点（IP+端口）
//  主机间通过端点标识自己
// 
inline int client_end_point();

inline int server_end_point();

//
// @brief 创建一个TCP Socket
//
inline int create_tcp_socket();

//
// @brief 服务器端用于接受客户端的连接请求
// 
inline int create_acceptor_socket();

//
// @brief Socket需要绑定IP和端口（即endpoint）来标识自己，才能通信
// 
inline int bind_acceptor_socket();

// 
// @brief 客户端连接服务器
// 
inline int connect_to_end();

//
// @brief 通过DNS域名连接服务器
//
inline int dns_connect_to_end();

//
// @服务器接受连接
//
inline int accept_new_connection();

// 
// @将数据转为Asio的buffer类型（存储到buffer）中
//
inline void use_const_buffer();

//
// 使用buffer函数直接std::string转换为buffer
//
inline void use_buffer_str();

//
// 将char*指针转为buffer
//
inline void use_buffer_arr();

// 通过write_some同步发送数据
inline void write_to_socket(asio::ip::tcp::socket & sock);

inline int send_data_by_write_some();

inline std::string read_from_socket(asio::ip::tcp::socket& sock);

inline int write_data_by_read_some();
#endif

