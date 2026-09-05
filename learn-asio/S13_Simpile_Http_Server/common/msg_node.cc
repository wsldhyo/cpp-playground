#include "msg_node.hpp"
#include "constant.hpp"
#include <cstring>
#include <asio/detail/socket_ops.hpp>
MsgNode::MsgNode(short _max_len) : cur_len(0), total_len(_max_len) {
  data = new char[total_len + 1];
  data[total_len] = '\0';
}

MsgNode::~MsgNode() { delete[] data; }

void MsgNode::clear() {
  memset(data, 0, total_len + 1);
  cur_len = 0;
}

RecvNode::RecvNode(short _max_len, short _msg_id)
    : MsgNode(_max_len), msg_id(_msg_id) {}

SendNode::SendNode(char const* _data, short _max_len, short _msg_id)
    : MsgNode(_max_len + HEAD_TOTAL_LENGTH), msg_id(_msg_id)
{
    short msg_id_net = asio::detail::socket_ops::host_to_network_short(_msg_id);
    memcpy(data, &msg_id_net, HEAD_ID_LENGTH);
    short max_len_net = asio::detail::socket_ops::host_to_network_short(_max_len);
    memcpy(data + HEAD_ID_LENGTH, &max_len_net, HEAD_DATA_LENGTH);
    memcpy(data + HEAD_TOTAL_LENGTH, _data, _max_len);
}