#ifndef LOGIC_SYSTEM_HPP
#define LOGIC_SYSTEM_HPP
#include <condition_variable>
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <map>
#include <memory>
#include <queue>
#include <thread>

class Session;
class RecvNode;
struct LogicNode {
  LogicNode(std::shared_ptr<Session> _session,
            std::shared_ptr<RecvNode> _recv_node);

  std::shared_ptr<Session> session;
  std::shared_ptr<RecvNode> recv_node;
};

class LogicSystem {
public:
  using FuncCallBackt_t =
      std::function<void(std::shared_ptr<Session> session, short const &msg_id,
                         std::string const &msg_data)>;
  LogicSystem();
  ~LogicSystem();
  void post_msg_to_que(std::shared_ptr<LogicNode> _msg);
  static LogicSystem &get_instance();
  LogicSystem(LogicSystem const &) = delete;
  LogicSystem &operator=(LogicSystem const &) = delete;

private:
  void deal_msg_();
  void register_callback_();
  void hello_world_callback_(std::shared_ptr<Session> session,
                             short const &msg_id, std::string const &msg_data);
  std::thread work_thread_;
  std::queue<std::shared_ptr<LogicNode>> msg_que_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool b_stop_;
  std::map<short, FuncCallBackt_t> func_callbacks_;
};
#endif