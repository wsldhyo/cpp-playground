#include "proto/demo.grpc.pb.h"
#include "proto/demo.pb.h"
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/status.h>
#include <iostream>
#include <memory>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;

class Client {
public:
  Client(std::shared_ptr<Channel> _channel)
      : stub_(Greeter::NewStub(_channel)) {}

 std::string SayHello(std::string name){
    ClientContext context;
    HelloReply reply;
    HelloRequest request;
    request.set_message(name);
    // RPC， 远端调用服务端的SayHello方法
    auto status = stub_->SayHello(&context, request, &reply);
    if (status.ok()) {
        return  reply.message();
    }
    else  {
        return "false " + status.error_message();
    }
 }
private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char *argv[]) {
    auto channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
    Client client(channel);
    std::string res = client.SayHello("Hello Server");
    std::cout << res << std::endl;
    return 0;
}