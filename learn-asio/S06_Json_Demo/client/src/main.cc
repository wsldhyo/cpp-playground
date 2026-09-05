// SyncClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <asio/error_code.hpp>
#include <asio/ip/address.hpp>
#include <asio/write.hpp>
#include <asio/read.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/detail/socket_ops.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
using namespace std;
using namespace asio::ip;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
int main()
{
	try {
		//创建上下文服务
		asio::io_context   ioc;
		//构造endpoint
		tcp::endpoint  remote_ep(asio::ip::make_address("127.0.0.1"), 10086);
		tcp::socket  sock(ioc);
		asio::error_code  error = asio::error::host_not_found; ;
		sock.connect(remote_ep, error);
		if (error) {
			cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
			return 0;
		}

		Json::Value root;
		root["id"] = 1001;
		root["data"] = "hello world";
		std::string request = root.toStyledString();
		size_t request_length = request.length();
		char send_data[MAX_LENGTH] = { 0 };
		//转为网络字节序
		int request_host_length = asio::detail::socket_ops::host_to_network_short(request_length);
		memcpy(send_data, &request_host_length, 2);
		memcpy(send_data + 2, request.c_str(), request_length);
		asio::write(sock, asio::buffer(send_data, request_length + 2));
		cout << "begin to receive..." << endl;
		char reply_head[HEAD_LENGTH];
		size_t reply_length = asio::read(sock, asio::buffer(reply_head, HEAD_LENGTH));
		short msglen = 0;
		memcpy(&msglen, reply_head, HEAD_LENGTH);
		//转为本地字节序
		msglen = asio::detail::socket_ops::network_to_host_short(msglen);
		char msg[MAX_LENGTH] = { 0 };
		size_t  msg_length = asio::read(sock, asio::buffer(msg, msglen));
		Json::Reader reader;
		reader.parse(std::string(msg,msg_length),root);
		std::cout << "msg id is " << root["id"] << " msg is " << root["data"] << endl;
		getchar();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
	return 0;
}


