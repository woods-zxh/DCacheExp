/** 
This is my final homework in my school
**/
// #include "cache/cacheClient.h"
#include "logic/logicSvr.h"
#include <iostream>


int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "usage: " << argv[0] << " <logicsSvr-num> <cacheSvr-num> "
              << std::endl;
    return -1;
  }

  //1. 初始化logicSvr,cacheSvr

  std::string ip = "127.0.0.1";
  unsigned int port = 8000;
  printf("0.0\n");
  LogicSvr logicSvr;
  logicSvr.Start(ip, port);
  
  // CacheSvr cacheSvr;
  // cacheSvr.Start(ip, port);

  // 2. 获得实时的观测数据

}
