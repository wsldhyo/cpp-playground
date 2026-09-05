#ifndef EPOLL_TCP_CHANNEL_HPP
#define EPOLL_TCP_CHANNEL_HPP

#include <cstdint>
struct Poller;
typedef struct ETcpChannel ETcpChannel;
typedef void (*event_callback)(ETcpChannel *etch);
struct ETcpChannel {
  int fd;           // Handle
  uint32_t events;  // 关注的事件
  uint32_t revents; // 实际发生的事件
  // 处理事件的回调
  event_callback read_callback;
  event_callback write_callback;
  event_callback error_callback;
  event_callback after_event_callback;
  // 该Channel属于哪个事件处理器，EventLoop借此找到Acceptor、Connection等对象来调用事件处理逻辑
  // 而C++中一般用对象成员、lambda等方式保存上下文
  void *owner;
};

void channel_handle_event(ETcpChannel *etch);

void channel_enable_reading(Poller *poller, ETcpChannel *ch);

void channel_disable_reading(Poller *poller, ETcpChannel *ch);

void channel_enable_writing(Poller *poller, ETcpChannel *ch);

void channel_disable_writing(Poller *poller, ETcpChannel *ch);

#endif // EPOLL_TCP_CHANNEL_HPP