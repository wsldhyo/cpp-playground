#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/system_error.hpp>
#include <cstddef>
#include <exception>
#include <set>
#include <memory>
#include <thread>
#include <iostream>
#include <memory>
#include <unistd.h>




using sock_t = asio::ip::tcp::socket;
using sp_sock_t = std::shared_ptr<sock_t>;

std::size_t constexpr MAX_LENGTH{1024};
std::set<std::shared_ptr<std::thread>>  g_threads_set;


// 与客户端的一个会话，进行数据的收发
void session(sp_sock_t sock){

    try {
    for ( ;;) {
        char data[MAX_LENGTH]{'\0'};
        asio::error_code ec;
        // 服务器最好不要用read，影响效率
        std::size_t read_len {sock->read_some(asio::buffer(data, MAX_LENGTH), ec)};
        if (ec == asio::error::eof) {
            // 对端关闭连接
            std::cout << "connection closed by peer\n";
            break;
        }
        else if(ec)
        {
            // 系统错误，异常退出
            throw asio::system_error(ec);
        }

        std::cout << "receive from " << sock->remote_endpoint().address().to_string() << std::endl;
        std::cout << "receive message is " << data << std::endl;
        // 回传数据给对方
        asio::write(*sock, asio::buffer(data, read_len));
    }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread " << e.what() << '\n' << std::endl;
    }    
}


void server(asio::io_context& ioc, unsigned short port_num){
    // acceptor用于接收客户端连接强求
    // 创建后即绑定到对应端口
    // 使用ipv4协议
    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num));
    for(;;){
        sp_sock_t sock = std::make_shared<asio::ip::tcp::socket>(ioc);
        acceptor.accept(*sock);
        // 每个线程维护一个和客户端的会话（socket连接）
        auto thread = std::make_shared<std::thread>(session, sock);
        g_threads_set.insert(thread); // 防止局部线程对象下次循环后被回收

    }
}

int main(int argc, char* argv[]){
    try {
        asio::io_context ioc;
        server(ioc, 10086);
        for (auto& t: g_threads_set) {
            // 主线程等待子线程执行完毕
            if (t->joinable()) {
                t->join();
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread " << e.what() << '\n' << std::endl;
    }
    return 0;
}