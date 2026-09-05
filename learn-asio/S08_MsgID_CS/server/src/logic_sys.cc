
#include <iostream>

#include <json/reader.h>
#include <json/value.h>
#include <memory>
#include <mutex>

#include "../../common/const.h"
#include "../include/logic_sys.hpp"
LogicNode::LogicNode(std::shared_ptr<CSession> session,
                     std::shared_ptr<RecvNode> recv_node)
    : session_(session), recv_node_(recv_node) {}

LogicSystem::LogicSystem() : b_stop_(false) {
  register_callback_();
  work_thread_ = std::thread(&LogicSystem::deal_msg_, this);
}

LogicSystem::~LogicSystem() {
  b_stop_ = true;
  cond_.notify_one();
  work_thread_.join();
}

void LogicSystem::register_callback_() {
  func_callbacks_[MSG_HELLO_WORLD] = std::bind(
      &LogicSystem::hello_world_callback_, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::hello_world_callback_(std::shared_ptr<CSession> session,
                                        short const &msg_id,
                                        std::string const &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  std::cout << "receive msg id is " << root["id"].asInt() << " msg data is "
            << root["data"].asString() << std::endl;

  root["data"] =
      "server has received  msg, msg data is " + root["data"].asString();
  std::string return_str = root.toStyledString();
  session->send(return_str, root["id"].asInt());
}

void LogicSystem::deal_msg_() {
  std::shared_ptr<LogicNode> msg_node;
  auto callback_iter = func_callbacks_.begin();
  while (true) {
    std::unique_lock<std::mutex> ulock(mutex_);

    // 队列为空则等待
    while (msg_que_.empty()) {
      cond_.wait(ulock);
    }

    // 如果为关闭状态，则取出逻辑队列所有数据, 处理完毕后退出
    if (b_stop_) {
      while (msg_que_.empty()) {
        msg_node = msg_que_.front();
        callback_iter = func_callbacks_.find(msg_node->recv_node_->msg_id);
        msg_que_.pop();
        if (callback_iter == func_callbacks_.end()) {
          continue;
        }
        callback_iter->second(msg_node->session_, msg_node->recv_node_->msg_id,
                              std::string(msg_node->recv_node_->data,
                                          msg_node->recv_node_->cur_len));
      }
      break;
    }

    // 如果没有停服，且队列中有数据
    msg_node = msg_que_.front();
    callback_iter = func_callbacks_.find(msg_node->recv_node_->msg_id);
    msg_que_.pop();
    if (callback_iter == func_callbacks_.end()) {
      continue;
    }
    callback_iter->second(
        msg_node->session_, msg_node->recv_node_->msg_id,
        std::string(msg_node->recv_node_->data, msg_node->recv_node_->total_len));
  }
}

void LogicSystem::post_msg_to_que(std::shared_ptr<LogicNode> msg_node) {
  std::unique_lock<std::mutex> ulock(mutex_);
  msg_que_.push(msg_node);
  if (msg_que_.size() == 1) {
    cond_.notify_one();
  }
}