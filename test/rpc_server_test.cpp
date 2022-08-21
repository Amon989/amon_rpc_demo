#include <thread>
#include "../include/rpc_server.h"

int add(int a, int b){
    return a+b;
}

int main() {
    try {
        asio::io_context io_context;
        std::thread thd([&io_context] {
            asio::io_context::work work(io_context);
            io_context.run();
        });
        amon::rpc_server server(io_context, 9980);
        server.register_handler("add", add);
        async_simple::coro::syncAwait(server.start());
        thd.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}