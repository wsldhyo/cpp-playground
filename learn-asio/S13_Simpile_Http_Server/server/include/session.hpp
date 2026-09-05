#ifndef SESSION_HPP
#define SESSION_HPP
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include "../../common/msg_node.hpp"
class Server;
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(asio::io_context& _io_context, Server* _server);
    ~Session();
    asio::ip::tcp::socket& get_socket();
    std::string& get_uuid();
    void start();
    void close();
    void send(char const* _msg, short _max_len, short _msg_id);
    void send(std::string const& msg, short _msg_id);
private:
    void write_callback_(asio::error_code _ec, std::shared_ptr<Session> _shared_this);
    asio::io_context& io_context_;
    Server* server_;
    asio::ip::tcp::socket socket_;
    std::string uuid_;
    bool b_close_;
    std::mutex mutex_;
    std::queue<std::shared_ptr<SendNode>> send_que_;
    std::shared_ptr<RecvNode>   recv_msg_node_;
    std::shared_ptr<MsgNode>    recv_head_node_;
};

#endif