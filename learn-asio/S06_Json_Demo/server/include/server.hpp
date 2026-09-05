#pragma once
#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include "session.hpp"
#include <memory.h>
#include <map>
using namespace std;
using asio::ip::tcp;
class CServer
{
public:
	CServer(asio::io_context& io_context, short port);
	void remove_session(std::string);
private:
	void accept_callback_(shared_ptr<CSession>, const asio::error_code& error);
	void start_accept_();
	asio::io_context &_io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, shared_ptr<CSession>> _sessions;
};