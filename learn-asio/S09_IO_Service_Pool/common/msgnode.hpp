#ifndef MSG_NODE_HPP
#define MSG_NODE_HPP

// MsgNode 解耦，拆分为接收消息节点和发送消息节点
struct MsgNode {
  explicit MsgNode(short max_len);

  ~MsgNode();

  void clear();

  short cur_len;
  short total_len;
  char *data;
};


struct RecvNode : public MsgNode
{
    RecvNode(short max_len, short msg_id);

    short msg_id;
};

struct SendNode : public MsgNode
{
    SendNode(char const* msg, short msg_len, short msg_id);

    short msg_id;
};
#endif