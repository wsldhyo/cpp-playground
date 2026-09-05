#ifndef SERVER_HPP
#define SERVER_HPP
#include "session.hpp"
#include <string>
#include <map>
#include <memory>
class ServerErr{
public:
    ServerErr(asio::io_context& ioc, short port_num);
private:
    void start_accept();
    // accept异步回调
    void handle_accept(SessionErr* new_session, asio::error_code const& ec);
    
private:
    asio::io_context& ioc_;
    asio::ip::tcp::acceptor acceptor_;
};


// 利用伪闭包延长生命周期
class CServer{
public:
    CServer(asio::io_context& ioc, short port_num);
    void remove_session(std::string& uuid); 
private:
    void start_accept();
    void handle_accept(std::shared_ptr<CSession> new_session, asio::error_code const& ec);
private:
    asio::io_context& ioc_;
    asio::ip::tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<CSession>> sessions_;
};

// CSession2增加异步发送队列实现全双工通信
class CServer2{
public:
    CServer2(asio::io_context& ioc, short port_num);
    void remove_session(std::string& uuid); 
private:
    void start_accept();
    void handle_accept(std::shared_ptr<CSession2> new_session, asio::error_code const& ec);
private:
    asio::io_context& ioc_;
    asio::ip::tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<CSession2>> sessions_;
};
#endif