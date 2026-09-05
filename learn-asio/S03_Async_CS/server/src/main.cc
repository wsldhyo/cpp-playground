#include "../include/server.hpp"
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
    try{
        asio::io_context ioc;
        //ServerErr s(ioc, 10086);
        CServer2 s(ioc, 10086);
        ioc.run(); // 启动事件循环
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}