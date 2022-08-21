//
// Created by amon on 8/19/22.
//

#ifndef ASYNC_SIMPLE_CONNECTION_H
#define ASYNC_SIMPLE_CONNECTION_H

#include "asio_util.h"
#include "router.h"
#include "codec.h"

namespace amon {

using asio::ip::tcp;

class connection : public std::enable_shared_from_this<connection> {
public:
    connection(asio::ip::tcp::socket socket, amon::router &_router)
        : socket_(std::move(socket)), router_(_router) {}
    ~connection() {
        std::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }

    friend router;

    async_simple::coro::Lazy<void> start() {
        for (;;) {
            auto [error, bytes_transferred] =
                co_await async_read_some(socket_, asio::buffer(read_buf_));
            if (error) {
                std::cout << "connection read error: " << error.message()
                          << ", size=" << bytes_transferred << '\n';
                break;
            }
            co_await router_.route<connection>(read_buf_, bytes_transferred, this -> shared_from_this());
        }
    }

private:

    async_simple::coro::Lazy<void> send_err(std::string msg){
        auto err = codec_.pack(msg);
        size_t len = err.size();
        memcpy(write_buf_, &len, sizeof(size_t));
        memcpy(write_buf_ + sizeof(size_t), err.data(), err.size());
        auto [error, dummy] = 
                co_await async_write(socket_, asio::buffer(write_buf_, len + sizeof(size_t)));
        if (error) {
            std::cout << "send error error: " << error.message() << '\n';
        }
    }


private:

    std::string parse_func_name(const char* data, int len){
        return codec_.unpack<std::string>(data, len);
    }
    int parse_len(const char* data){
        return codec_.unpack<unsigned int>(data, 4);
    }

    asio::ip::tcp::socket socket_;
    char read_buf_[1024];
    char write_buf_[1024];
    router &router_;
    codec codec_;
};

}

#endif  // ASYNC_SIMPLE_CONNECTION_H
