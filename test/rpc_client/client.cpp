#include <iostream>
#include <string>
#include "rpc_client_64.h"

class foo2{
public:
    void set_data(int d){
        data = d;
    }
    int get_data()const{
        return data;
    }
    /*声明需要序列化的成员变量*/
    MSGPACK_DEFINE(data);
private:
    int data;
};

std::ostream& operator<<(std::ostream& out, const foo2& obj){
    out << obj.get_data();
    return out;
}

int main(int argc, char** argv){
    auto client = std::make_shared<rpcClient>("127.0.0.1",9980);
    client->connect();
    try{
        int res = client->call<int>(std::string("add"),1,2);
        std::cout << res << std::endl;

        // double res2 = client->call<double>("div",1,2);
        // std::cout << res2 << std::endl;
        
        // foo2 obj;
        // res = client->call<int>("func",obj,10);
        // std::cout << res << std::endl;
        
        // obj = client->call<foo2>("get_foo2",20);
        // std::cout << obj << std::endl;

    }catch(std::string err){
        std::cerr << err << std::endl;
    }
    return 0;
}
