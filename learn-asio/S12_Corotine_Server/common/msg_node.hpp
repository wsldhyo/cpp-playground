#ifndef MSG_NODE_HPP
#define MSG_NODE_HPP
#include "constant.hpp"
class MsgNode {
public:
  MsgNode(short _max_len);
  ~MsgNode();

  void clear();

  short cur_len;  // 当前已经处理的长度
  short total_len; // data的总长度
  char *data;
};

class RecvNode : public MsgNode {
public:
    RecvNode(short _max_len, short _msg_id);

    short msg_id;
};

class SendNode : public MsgNode {
public:
    SendNode(char const* _msg, short _max_len, short _msg_id);

    short msg_id;
};

#endif