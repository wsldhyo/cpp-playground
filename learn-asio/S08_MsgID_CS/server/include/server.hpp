#ifndef SERVER_HPP
#define SERVER_HPP
#include "session.hpp"
#include <asio/error_code.hpp>
#include <asio/system_error.hpp>
#include <string>
#include <map>
#include <memory>

#include <asio.hpp>
#include "../include/session.hpp"
#include <memory.h>
#include <map>

class CServer
{
public:
	CServer(asio::io_context& io_context, short port);
	void remove_session(std::string);
private:
	void accept_callback_(std::shared_ptr<CSession>, const asio::error_code & error);
	void start_accept_();
	asio::io_context &_io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};
#endif