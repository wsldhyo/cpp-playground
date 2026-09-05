#ifndef CONST_HPP
#define CONST_HPP 
inline constexpr int MAX_LENGTH  {1024*2};
inline constexpr int HEAD_LENGTH {2};

// 限制队列上限，避免单个会话的队列占用过多资源，影响其他的会话
inline constexpr int MAX_RECVQUE { 10000};
inline constexpr int MAX_SENDQUE {1000};
#endif 