
#include <iostream>
#include <time.h>
#include<sys/time.h>

#include "../cache/cacheClient.h"
#include "../storage/storageClient.h"
#include "logicSvr.h"

LogicSvr::LogicSvr() {
    std::string ip = "127.0.0.1";
    unsigned int port = 8000;
    Start(ip, port);
}

LogicSvr::~LogicSvr() {}

int LogicSvr::Start(const std::string ip, unsigned int port) {
    //创建一个新的进程
    int ret = 0;

    //读取多集群配置
    ret = LoadConfig();

    //处理逻辑
    ret = Process();
    if (ret != 0) {
        return ret;
   }
    return -1;
}

int LogicSvr::LoadConfig() {
    
}


std::string LogicSvr::GenerateKey(unsigned long long seed) {
    //TODO
    return "todo";
}

int LogicSvr::GenerateOp(unsigned long long seed) {
    //1 get 2 set 3 mget 4 mset
    return 1;
}


int LogicSvr::Process() {
    int ret = 0;

    //初始化以及连接
    CacheClient cacheClient;
    StorageClient storageClient;

    // cacheClient.Connect("127.0.0.1", 7000);

    unsigned long long seed = 0;
    int opFlag = true;
    int num = 0;
    while(num <= 0) {
        std::string key = GenerateKey(seed);
        std::string value;
        unsigned int cost;
        opFlag = GenerateOp(seed);

        if (opFlag == 1) {
            struct timeval start;
            struct timeval end;
            float time_use = 0;
            gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样

            key = "wood";
            // for (int i = 0; i < 100; i++) {
                ret = cacheClient.Get(key, value, cost);
            // }
            gettimeofday(&end,NULL);
            time_use = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
            // printf("time_use is %.10f\n",time_use/1000);

            //cache不存在
            if (ret == 1) {
                ret = storageClient.Get(key, value, cost);
            }
            
            // std::cout << "value:" << value << std::endl;
        } else if (opFlag == 2) {
            unsigned int timeValue = time(NULL);
            value = std::to_string(timeValue);

            ret = storageClient.Set(key, value, cost);

            ret = cacheClient.Set(key, value, cost);
        } else if (opFlag == 3) {
            struct timeval start;
            struct timeval end;
            float time_use = 0;
            gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样
            // printf("start.tv_sec:%d\n",start.tv_sec);
            // printf("start.tv_usec:%d\n",start.tv_usec);

            std::vector<std::string> keyList;
            std::map<std::string, std::string> GetMap;
            for (int i = 0; i < 100; i++) {
                keyList.push_back("wood");
            }
            for (int i = 0; i < 100; i++) {
                keyList.push_back("hzl");
            }
            // keyList.push_back("wood");
            // keyList.push_back("woods");
            // keyList.push_back("handy");
            ret = cacheClient.ThreadPoolBatchGet(keyList, GetMap, cost);

            gettimeofday(&end,NULL);
            // printf("end.tv_sec:%d\n  ",end.tv_sec);
            // printf("end.tv_usec:%d\n",end.tv_usec);
            
            time_use = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
            printf("time_use is %.10f\n",time_use/1000);

            // std::cout << ret << " " << GetMap["woods"] << " " << GetMap["wood"] << " " << GetMap["handy"] << std::endl;

            // break;
        } else if (opFlag == 4) {

        }
        seed ++;
        num++;
        
    }   

    // //释放资源
    // cacheClient.DisConnect();

    return ret;
}