#ifndef LOGIC_SYS_HPP
#define LOGIC_SYS_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <string>

#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

#include "../../common/singleton.hpp"
#include "../../common/msgnode.hpp"
#include "session.hpp"


using ls_callback = std::function<void(std::shared_ptr<CSession> session, short const& msg_id, std::string const& msg_data)>;

class LogicSystem;
class LogicNode
{
public:
    friend class LogicSystem;

    LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node);
private:
    std::shared_ptr<CSession> session_;
    std::shared_ptr<RecvNode> recv_node_;

};


class LogicSystem : public Singleton<LogicSystem>
{
public:
    using msg_ID_t = short;    
    
    ~LogicSystem();
    void post_msg_to_que(std::shared_ptr<LogicNode> msg);
private:
    friend class Singleton<LogicSystem>;
    void register_callback_();
    void hello_world_callback_(std::shared_ptr<CSession>, short const& msg_id, std::string const& msg_data);
    void deal_msg_();
    LogicSystem();

    std::queue<std::shared_ptr<LogicNode>> msg_que_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread work_thread_;
    bool b_stop_;
    std::map<msg_ID_t, ls_callback> func_callbacks_;
};



#endif