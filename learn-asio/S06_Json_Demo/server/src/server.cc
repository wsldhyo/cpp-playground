#include "../include/server.hpp"
#include <iostream>
CServer::CServer(asio::io_context& io_context, short port):_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(),port))
{
	cout << "Server start success, listen on port : " << _port << endl;
	start_accept_();
}

void CServer::accept_callback_(shared_ptr<CSession> new_session, const asio::error_code& error){
	if (!error) {
		new_session->start();
		_sessions.insert(make_pair(new_session->get_uuid(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.message() << endl;
	}

	start_accept_();
}

void CServer::start_accept_() {
	shared_ptr<CSession> new_session = make_shared<CSession>(_io_context, this);
	_acceptor.async_accept(new_session->get_socket(), std::bind(&CServer::accept_callback_, this, new_session, placeholders::_1));
}

void CServer::remove_session(std::string uuid) {
	_sessions.erase(uuid);
}