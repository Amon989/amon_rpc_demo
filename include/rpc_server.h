//
// Created by amon on 8/18/22.
//

#ifndef RPC_DEMO_RPC_SERVER_H
#define RPC_DEMO_RPC_SERVER_H

#include <iostream>
#include <thread>

#include "asio_util.h"
#include "router.h"
#include "connection.h"

namespace amon {

using asio::ip::tcp;

class rpc_server {
public:
    rpc_server(asio::io_context& io_context, unsigned short port)
        : io_context_(io_context), port_(port), executor_(io_context) {}

    async_simple::coro::Lazy<void> start() {
        tcp::acceptor a(io_context_, tcp::endpoint(tcp::v4(), port_));
        for (;;) {
            tcp::socket socket(io_context_);
            auto error = co_await async_accept(a, socket);
            if (error) {
                std::cout << "Accept failed, error: " << error.message()
                          << '\n';
                continue;
            }
            std::cout << "New client comming.\n";
            start_one(std::move(socket)).via(&executor_).detach();
        }
    }

    async_simple::coro::Lazy<void> start_one(asio::ip::tcp::socket socket) {
        auto conn = std::make_shared<connection>(std::move(socket), router_);
        co_await conn -> start();
    }

    template <typename Function>
    void register_handler(std::string const &name, const Function &f) {
        router_.register_handler(name, std::forward<Function>(f));
    }

private:
    asio::io_context& io_context_;
    std::unordered_map<std::string,
        std::function<void(const char* data, int len)>> m_handlers;
    unsigned short port_;
    AsioExecutor executor_;
    router router_;
};

}

#endif // RPC_DEMO_RPC_SERVER_H