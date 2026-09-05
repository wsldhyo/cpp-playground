#ifndef NONCP_NONMV_HPP
#define NONCP_NONMV_HPP
class NonCopyable{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const &) = delete;
};

class NonMovable{
protected:
    NonMovable() = default;
    ~NonMovable() = default;
    NonMovable(NonMovable &&) = delete;
    NonMovable& operator=(NonMovable &&) = delete;
};
#endif // NONCP_NONMV_HPP