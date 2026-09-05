#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <map>
#include <memory>
#include <asio/error_code.hpp>
#include <asio/system_error.hpp>
#include <asio/io_context.hpp>
#include "session.hpp"
class CServer
{
public:
	CServer(asio::io_context& io_context, short port);
	void remove_session(std::string);
private:
	void accept_callback_(std::shared_ptr<CSession>, const asio::error_code & error);
	void start_accept_();
	short port_;
	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	std::map<std::string, std::shared_ptr<CSession>> sessions_;
};
#endif