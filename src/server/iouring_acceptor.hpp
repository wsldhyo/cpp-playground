#ifndef IOURING_ACCEPTOR_HPP
#define IOURING_ACCEPTOR_HPP
struct IOService;
struct Request;
typedef struct Acceptor {
  IOService *service;
  int listen_fd;
} Acceptor;

void acceptor_init(Acceptor *acceptor, IOService *service);
void acceptor_destroy(Acceptor *acceptor);
int acceptor_start(Acceptor *acceptor);
void acceptor_handle_completion(Request *req, int result);

#endif // IOURING_ACCEPTOR_HPP