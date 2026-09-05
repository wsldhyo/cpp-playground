#include "proto/demo.grpc.pb.h"
#include "proto/demo.pb.h"
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <memory>
using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;

class GreeterServiceImpl final : public Greeter::Service {
public:
  grpc::Status SayHello(grpc::ServerContext *context,
                        const HelloRequest *request,
                        HelloReply *response) override {
    std::string prefix("ldh grpc server has received:");
    response->set_message(prefix + request->message());
    return grpc::Status::OK;
  }
};

void run_server() {
  std::string server_addr("0.0.0.0:50051");
  GreeterServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_addr << std::endl;
  server->Wait();
}

int main(int argc, char *argv[]) {
  run_server();
  std::cout << "Hello Wrold" << std::endl;
  return 0;
}