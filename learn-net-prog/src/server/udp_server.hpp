#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP
#include <cstdint>
#include <netinet/in.h>

class UdpServer {
public:
    UdpServer(uint16_t port);
    ~UdpServer();
    void start();
private:
    void create_sock();
    void bind(uint16_t port);
    
    int32_t sock_;
    sockaddr_in serv_addr_;
};
#endif // UDP_SERVER_HPP