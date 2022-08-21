//
// Created by amon on 8/19/22.
//

#ifndef ASYNC_SIMPLE_CODEC_H
#define ASYNC_SIMPLE_CODEC_H

#include <msgpack.hpp>
#include <tuple>
#include <string>
#include <iostream>
#include "function_traits.h"

class codec final{
public:
    codec() = default;
    ~codec() = default;
    using buffer_type = msgpack::sbuffer;
    const int static init_size = 2048;


    /****************************parsingUtil*********************************/

    std::string parse_func_name(const char* data, int len){
        return unpack<std::string>(data, len);
    }
    int parse_len(const char* data){
        return unpack<unsigned int>(data, sizeof(int));
    }


    /****************************packing*********************************/

    // 将特定类型的内容打包为msgpack::pack类型
    template <typename T>
    buffer_type pack(T &&t){
        buffer_type buffer;
        msgpack::pack(buffer, std::forward<T>(t));
        return buffer;
    }

    // 将函数名string和参数一同打包为msgpack::pack类型
    template <typename... Args>
    buffer_type pack_args(const std::string funcName, Args &&...args) {
        buffer_type buffer(init_size);
        auto t = std::forward_as_tuple(args...);
        msgpack::pack(buffer, std::tuple<const std::string, decltype(t)>{funcName, t});
        return buffer;
    }
    
    // 将参数打包为string
    template <typename Arg, typename... Args,typename = typename std::enable_if<std::is_enum<Arg>::value>::type>
    static std::string pack_args_str(Arg arg, Args &&...args) {
        buffer_type buffer(init_size);
        msgpack::pack(buffer,
                      std::forward_as_tuple((int)arg, std::forward<Args>(args)...));
        return std::string(buffer.data(), buffer.size());
    }

    template <typename T>
    void pack_succ_response(T &&t, std::string &s){
        buffer_type buffer = pack(std::forward<T>(t));
        unsigned int msg_code = 0;
        unsigned int len = buffer.size();   

        s.resize(sizeof(unsigned int) * 2 + buffer.size());
        char* xs = (char*)s.data();

        std::memcpy(xs, &msg_code, sizeof(unsigned int));
        std::memcpy(xs + sizeof(unsigned int), &len, sizeof(unsigned int));
        std::memcpy(xs + 2 * sizeof(unsigned int), buffer.data(), len);
    }

    // 构建错误包
    void pack_fail_response(const char *err, std::string &s){
        if (err == nullptr) {
            //unsigned int len = s.size();
            std::string tmp = "\0\0\0\0";
            s = tmp + s;
        } else {
            unsigned int len = strlen(err);   

            s.resize(sizeof(unsigned int) + len);
            char* xs = (char*)s.data();

            std::memcpy(xs, &len, sizeof(unsigned int));
            std::memcpy(xs + sizeof(unsigned int), err, len);
        }
    }

    /****************************unpacking*********************************/

    // 入参为msgpack::sbuffer解出函数名turple
    std::tuple<std::string> unpack_func_name(const buffer_type& data){
        try{
            msgpack::unpack(msg_,data.data(), data.size());
            return msg_.get().as<std::tuple<std::string>>();
        }catch(...){
            std::cout << "函数名参数不匹配" << std::endl;
            return "";
        }
    }

    // 入参为msgpack::sbuffer解出函数名外加参数的turple
    template<typename T>
    std::tuple<const std::string, T> unpack(const buffer_type& data){
        try{
            msgpack::unpack(msg_,data.data(),data.size());
            return msg_.get().as<std::tuple<const std::string, T>>();
        }catch(...){
            std::cout << "参数类型不匹配" << std::endl;
            return std::tuple<const std::string, T>();
        }
    }

    // 解出特定类型
    template<typename T>
    T unpack(const char* data, const unsigned int length){
        try{
            msgpack::unpack(msg_, data, length);
            return msg_.get().as<T>();
        }catch(...){
            std::cout << "Args not match" << std::endl;
            return T();
        }
    }

    template <typename T> T unpack_throw(char const *data, unsigned int length) {
        try {
            msgpack::unpack(msg_, data, length);
            return msg_.get().as<T>();
        } catch (...) {
            throw std::invalid_argument("unpack failed: Args not match!");
        }
    }

    // 解出特定类型带err
    template<typename T>
    T unpack(const char* data, const unsigned int length, int& err){
        try{
            msgpack::unpack(msg_,data,length);
            err = 0;
            return msg_.get().as<T>();
        }catch(...){
            err = 1;
            return T();
        }
    }

public:
    msgpack::unpacked msg_;
};

#endif  // ASYNC_SIMPLE_CODEC_H
