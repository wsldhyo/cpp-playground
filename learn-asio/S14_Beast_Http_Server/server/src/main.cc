#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/detail/error_category.hpp>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <google/protobuf/json/json.h>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <memory>
#include <string>
namespace beast = boost::beast;
namespace http = beast::http;
namespace ip = boost::asio::ip;
namespace asio = boost::asio;
using tcp = ip::tcp;
namespace MyState {
std::size_t request_count() {
  static std::size_t count = 0;
  return ++count;
}

std::time_t now() { return std::time(0); }

} // namespace MyState

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
  HttpConnection(tcp::socket _socket) : socket_(std::move(_socket)) {}

  void start() {
    read_requset();
    check_deadline();
  }

  void read_requset() {
    auto self = shared_from_this();
    // 捕获self，延长this生命后期，回调时需要访问this的成员
    http::async_read(socket_, buffer_, request_,
                     [self](auto _ec, auto _transferred_bytes) {
                       if (!_ec) {
                         self->process_request();
                       }
                     });
  }

  /**
   * @brief 连接超时检测
   *
   */
  void check_deadline() {
    auto self = shared_from_this();
    // self传入回调，延长生命周期，避免回调时this已经被析构，导致回调内部错误
    deadline_.async_wait([self](auto _ec) {
      // 超时时关闭socket——
      if (!_ec) {
        self->socket_.close(_ec);
      }
    });
  }

  void process_request() {
    
    response_.version(request_.version()); // 保证响应的http版本与请求版本一样
    response_.keep_alive(false); // 短http连接
    switch (request_.method()) {
    case http::verb::get:
      response_.result(http::status::ok);
      // 构造http响应体。
      response_.set(http::field::server, "Beast"); //  告知对方的服务器类型
      create_response();
      break;
    case http::verb::post:
      response_.result(http::status::ok);
      response_.set(http::field::server, "Beast");
      create_post_response();
      break;
    default:
      // 不处理其他类型的请求
      response_.result(http::status::bad_request);
      // 构造http响应体。
      response_.set(http::field::content_type, "text/plain"); // 纯文本类型
      // 响应的消息体
      beast::ostream(response_.body())
          << "Invalid request-method'" << std::string(request_.method_string())
          << "'";
      break;
    }
    write_response();
  }

  // 根据请求的路由，构造不同的响应消息
  void create_response() {

    if (request_.target() == "/count") {
      // 返回一个html网页
      response_.set(http::field::content_type, "text/html");
      beast::ostream(response_.body())
          << "<html>\n"
          << "<head><title>Request count</title></head>\n"
          << "<body>\n"
          << "<h1>Request count</h1>\n"
          << "<p>There have been " << MyState::request_count()
          << " requests so far.</p>\n"
          << "</body>\n"
          << "</html>\n";
    } else if (request_.target() == "/time") {
      // 返回一个显示时间的html网页
      response_.set(http::field::content_type, "text/html");
      beast::ostream(response_.body())
          << "<html>\n"
          << "<head><title>Current time</title></head>\n"
          << "<body>\n"
          << "<h1>Current time</h1>\n"
          << "<p>The current time is " << MyState::now()
          << " seconds since the epoch.</p>\n"
          << "</body>\n"
          << "</html>\n";
    } else {
      // 路由不存在，404错误
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body()) << "File not found\r\n";
    }

  }

  void write_response() {
    auto self = shared_from_this();
    response_.content_length(
        response_.body().size()); // 给出包体大小，以便浏览器切包

    http::async_write(socket_, response_, [self](auto _ec, auto _size) {
      // 短连接，发送完就结束连接
      // 四次挥手，服务器先关闭socket的发送端
      // 服务器不要关闭socket连接，避免僵尸连接
      self->socket_.shutdown(tcp::socket::shutdown_send, _ec);
      // 会话结束，无需再进行超时检查
      self->deadline_.cancel();
    });
  }

  void create_post_response() {
    if (request_.target() == "/email") {
      auto &body = this->request_.body();
      auto body_str = beast::buffers_to_string(body.data());
      std::cout << "Receive data is " << body_str << std::endl;
      this->response_.set(http::field::content_type, "text/json");
      Json::Value response_root;
      Json::Reader reader;
      Json::Value src_root;
      bool parse_success = reader.parse(body_str, src_root);
      if (!parse_success) {
        std::cout << "Falied to parse Json data" << std::endl;
        response_root["error"] = 1001;
        std::string json_str = response_root.toStyledString();
        beast::ostream(response_.body()) << json_str;
        return;
      }
      auto email = src_root["email"].asString();
      std::cout << "email address is " << email << std::endl;
      response_root["error"] = 0;
      response_root["email"] = src_root["email"];
      response_root["msg"] = "receive eamil post success";
      std::string json_str = response_root.toStyledString();
      beast::ostream(response_.body()) << json_str;
    } else {
      // 其他post请求不处理，统一回复not found
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body()) << "File not found\r\n";
    }
  }

private:
  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  http::request<http::dynamic_body>
      request_; // dynamic_body，允许对方发起多种http请求。
                // 例如纯文本请求、文件请求等
  http::response<http::dynamic_body> response_;
  // 超时时间，一个http请求最多处理一分钟
  asio::steady_timer deadline_{socket_.get_executor(),
                               std::chrono::seconds(60)};
};
void http_server(tcp::acceptor &_acceptor, tcp::socket &_socket) {
  _acceptor.async_accept(_socket, [&](auto _ec) {
    if (!_ec) {
      std::make_shared<HttpConnection>(std::move(_socket))->start();
      http_server(_acceptor, _socket);
    }
  });
}

int main(int argc, char *argv[]) {
  try {
    auto const address = ip::make_address("127.0.0.1");
    unsigned short port = static_cast<unsigned short>(8080);
    asio::io_context ioc{1};
    tcp::acceptor acceptor{ioc, {address, port}};
    tcp::socket socket{ioc};
    http_server(acceptor, socket);
    ioc.run();
  } catch (std::exception &e) {
    std::cerr << "Error " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
