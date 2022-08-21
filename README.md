# amon_rpc_demo

## 概述

基于c++20的rpc demo，[async_simple](https://github.com/alibaba/async_simple)作为协程库，asio用于网络io，[msgpack](https://github.com/msgpack/msgpack-c)用于传输内容的序列化和反序列化。

## 环境
只能说一跟协程扯上关系就很麻烦：
- 本机linux版本：Ubuntu 20.04
- 本机所使用工具链：
    - cmake： version 3.16.3
    - clang++-12： version 12.0.0-3ubuntu1~20.04.5
- 本机boost版本： libboost1.71-dev:amd64
- 需安装async_simple、msgpack

## 使用
- 如果用vscode remote-ssh打开的话，环境没问题应该就能直接编译运行了
- 命令行
    - cmake & make

## 特别感谢
- [rest_rpc](https://github.com/qicosmos/rest_rpc)
- [rpc_demo](https://github.com/humeng1832596901/rpc_demo)

