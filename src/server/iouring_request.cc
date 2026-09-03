#include "iouring_request.hpp"

#include <cstdlib>
#include <cstring>

Request *request_create(OpType type) {
  Request *req = (Request *)malloc(sizeof(Request));
  if (!req)
    return NULL;
  memset(req, 0, sizeof(Request));
  req->type = type;
  req->fd = -1;
  return req;
}

void request_destroy(Request *req) { free(req); }