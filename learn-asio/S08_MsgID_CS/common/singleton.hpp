#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <mutex>
template <typename  T>
class Singleton 
{
public:
    ~Singleton() = default;
    static T* get_instance(){
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){

            instance_ = new T() ;
        });
    
        return instance_;
    }
protected:
    Singleton() = default;
    Singleton(T const&) = delete;
    Singleton& operator=(T const&) = delete;
    static T* instance_;
};

template <typename T>
T* Singleton<T>::instance_ = nullptr;
#endif