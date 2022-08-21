//
// Created by amon on 8/19/22.
//

#ifndef ASYNC_SIMPLE_ROUTER_H
#define ASYNC_SIMPLE_ROUTER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "codec.h"

namespace amon {

class connection;

template<typename F, size_t ...i, typename T>
decltype(auto) call_func_helper(F&& func, std::index_sequence<i...>, T&& t){
    return func(std::get<i>(std::forward<T>(t))...);
}

template<typename F, typename T>
decltype(auto) call_func(F&& func, T&& t){
    constexpr auto size = std::tuple_size<typename std::decay<T>::type>::value;
    return call_func_helper(std::forward<F>(func), std::make_index_sequence<size>{}, std::forward<T>(t));
}

class router {
public:
    // 注册函数
    template<typename F>
    void register_handler(const std::string funcName, F &handler){
        register_nonmember_func(funcName, handler);
    }

    template <typename T>
    async_simple::coro::Lazy<void> route(const char *data, 
            std::size_t size, std::weak_ptr<T> conn) {
        auto conn_sp = conn.lock();
        if (!conn_sp) {
            co_return;
        }

        codec codec_;
        std::string result;
        size_t begin = 0;

        for (size_t cur = begin; cur < size; cur += 1, begin = cur) {
            if (size - begin > sizeof(size_t)){
                size_t funcNameLen = codec_.unpack<size_t>(data + cur, sizeof(size_t));
                cur += sizeof(size_t);
                //解出函数名
                std::string funcName = codec_.unpack<std::string>(data + cur, funcNameLen);

                auto it = m_handlers.find(funcName);
                if (it == m_handlers.end()){
                    //构造错误包
                    std::string err_msg = std::string("The function ") + funcName
                                            + std::string(" has not been registered");
                    codec_.pack_fail_response(nullptr, err_msg);
                    
                    co_await async_write(conn_sp -> socket_, asio::buffer(err_msg));
                    co_return;
                }

                cur += funcNameLen;
                std::cout << "from: " << conn_sp -> socket_.remote_endpoint().address().to_string() << ":"
                            << conn_sp -> socket_.remote_endpoint().port() << ", call for function \'"
                            << funcName << "\'" << std::endl;

                
                size_t argsLen = codec_.unpack<size_t>(data + cur, sizeof(size_t));
                cur += sizeof(size_t);
                it -> second(data + cur, argsLen, result);

                co_await async_write(conn_sp -> socket_, asio::buffer(result));
                cur += argsLen;
                begin = cur;
            }
        }
    }
 
private:

    template<typename Function>
    void callproxy(Function func, const char* data, int len, std::string &result){
        using args_tuple = typename function_traits<Function>::tuple_type;
        codec codec;
        
        try {
            auto tp = codec.unpack_throw<args_tuple>(data, len);
            auto r = call_func(func, tp);
            codec.pack_succ_response(r, result);
        } catch (std::invalid_argument &e) {
                codec.pack_fail_response(e.what(), result);
            } catch (const std::exception &e) {
                codec.pack_fail_response(e.what(), result);
        }
    }

    template <typename Function> 
    struct invoker {

        static inline void apply(const Function &func,
                             std::weak_ptr<connection> conn, const char *data,
                             size_t len, std::string &result) {
            using args_tuple = typename function_traits<Function>::tuple_type;

            codec codec;
            try {
                auto tp = codec.unpack_throw<args_tuple>(data, len);
                call_func(func, tp);
            } catch (std::invalid_argument &e) {
                codec.pack_fail_response(e.what(), result);
            } catch (const std::exception &e) {
                codec.pack_fail_response(e.what(), result);
            }
        }
    };

    template <typename Function>
    void register_nonmember_func(const std::string &name, Function &f) {
        m_handlers[name] = std::bind(&router::callproxy<Function>, this, f, std::placeholders::_1, 
            std::placeholders::_2, std::placeholders::_3);
    }


private:
    std::unordered_map<std::string, std::function<void(const char*, int, std::string &)>> m_handlers;
};

} // namespace amon

#endif  // ASYNC_SIMPLE_ROUTER_H
