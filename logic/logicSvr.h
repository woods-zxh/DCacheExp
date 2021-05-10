#pragma once

#include <string>


class LogicSvr {
 public:

    LogicSvr();

    ~LogicSvr();

    //启动函数
    int Start(const std::string ip, unsigned int port);

 private:
      //处理逻辑
    int Process();

    std::string GenerateKey(unsigned long long seed);

    int GenerateOp(unsigned long long seed);

    int LoadConfig();

 private:
   


};