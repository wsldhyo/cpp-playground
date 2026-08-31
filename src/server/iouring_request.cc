#include "iouring_request.hpp"

#include <cstdlib>

Request *request_create(OpType type) {
  Request *req = (Request *)malloc(sizeof(Request));
  if (!req)
    return NULL;
  req->type = type;
  req->loop = NULL;
  req->owner.conn = NULL;
  req->fd = -1;
  req->buf = NULL;
  req->len = 0;
  return req;
}

void request_destroy(Request *req) { free(req); }