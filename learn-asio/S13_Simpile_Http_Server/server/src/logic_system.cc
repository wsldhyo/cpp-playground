#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

#include <json/reader.h>
#include <json/value.h>

#include "../include/logic_system.hpp"
#include "../../common/msg_node.hpp"
#include "../include/session.hpp"
LogicNode::LogicNode(std::shared_ptr<Session> _session,
                     std::shared_ptr<RecvNode> _recv_node)
    : session(_session), recv_node(_recv_node) {}

LogicSystem::LogicSystem(): b_stop_(false){
    register_callback_();
    work_thread_ = std::thread(&LogicSystem::deal_msg_, this);
}

LogicSystem::~LogicSystem() {
  b_stop_ = true;
  cond_.notify_one();
  work_thread_.join();
}


void LogicSystem::post_msg_to_que(std::shared_ptr<LogicNode> _msg) {
  std::unique_lock<std::mutex> lock(mutex_);
  msg_que_.push(_msg);
  if (msg_que_.size() >= 1) {
    cond_.notify_one();
  }
}

LogicSystem & LogicSystem::get_instance()
{
    static LogicSystem instance;
    return instance;
}

void LogicSystem::deal_msg_() {
  while (true) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() { return !msg_que_.empty() || b_stop_; });

    if (b_stop_) {
      while (!msg_que_.empty()) {
        auto msg_node = msg_que_.front();
        std::cout << "recv msg id is " << msg_node->recv_node->msg_id
                  << std::endl;
        auto callback_iter = func_callbacks_.find(msg_node->recv_node->msg_id);
        if (callback_iter == func_callbacks_.end()) {
          msg_que_.pop();
          continue;
        }
        callback_iter->second(msg_node->session, msg_node->recv_node->msg_id,
                              std::string(msg_node->recv_node->data,
                                          msg_node->recv_node->total_len));
        msg_que_.pop();
      }
      break;
    }

    auto msg_node = msg_que_.front();
    std::cout << "recv msg id is " << msg_node->recv_node->msg_id << std::endl;
    auto callback_iter = func_callbacks_.find(msg_node->recv_node->msg_id);
    if (callback_iter == func_callbacks_.end()) {
      msg_que_.pop();
      continue;
    }
    callback_iter->second(
        msg_node->session, msg_node->recv_node->msg_id,
        std::string(msg_node->recv_node->data, msg_node->recv_node->total_len));
    msg_que_.pop();
  }
}
void LogicSystem::register_callback_() {
  func_callbacks_[MSG_HEELO_WORLD] = std::bind(
      &LogicSystem::hello_world_callback_, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::hello_world_callback_(std::shared_ptr<Session> session,
                                        short const &msg_id,
                                        std::string const &msg_data) {

  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
  root["data"] =  "server has received msg, msg data is " + root["data"].asString();
  std::string const& return_str = root.toStyledString();
  session->send(return_str, root["id"].asInt());
}