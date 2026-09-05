#ifndef IOURING_IO_SERVICE_HPP
#define IOURING_IO_SERVICE_HPP
#include <liburing.h>
#include "iouring_io_backend.hpp"
struct Request;
typedef struct IOService {
    IOBackend io_backend;
    int running;
} IOService;
int io_service_init(IOService *service);

void io_service_run(IOService *service);

void io_service_stop(IOService *service);

void io_service_dispatch(IOService *service, struct Request *req, int result);

void io_service_destroy(IOService *service);

#endif // IOURING_IO_SERVICE_HPP