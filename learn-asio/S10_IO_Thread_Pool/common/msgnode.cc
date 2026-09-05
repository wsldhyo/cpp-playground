
#include "msgnode.hpp"
#include "const.h"
#include <asio/detail/socket_ops.hpp>
#include <cstring>
MsgNode::MsgNode(short max_len) : total_len(max_len), cur_len(0) {
  data = new char[total_len + 1];
  data[total_len] = '\0';
}

MsgNode::~MsgNode() { delete[] data; }

void MsgNode::clear() {
  ::memset(data, 0, total_len);
  data[total_len] = '\0';
  cur_len = 0;
}

RecvNode::RecvNode(short max_len, short msg_id_)
    : MsgNode(max_len), msg_id(msg_id_) {}

SendNode::SendNode(char const *msg, short msg_len, short msg_id_)
    : MsgNode(msg_len + MSG_DATA_LEN + MSG_ID_LEN), msg_id(msg_id_) {
        //先发送ID，然后是消息体长度，最后是消息体
        short id_netorder =  asio::detail::socket_ops::host_to_network_short(msg_id);
        short len_netorder = asio::detail::socket_ops::host_to_network_short(msg_len);
        memcpy(data, &id_netorder, MSG_ID_LEN);
        memcpy(data + MSG_ID_LEN, &len_netorder, MSG_DATA_LEN);
        memcpy(data + MSG_ID_LEN + MSG_DATA_LEN, msg , msg_len);
    }