#pragma once
#include <string>
#include <hiredis.h>
#include <vector>
#include <map>
#include <utility>
#include "util/threadpool.h"

struct NodeInfo {
   std::string ip;
   unsigned int port;
   bool isMaster;
   std::string hashValue;
   std::string masterHashValue;
};

class CacheClient {
 public:

    CacheClient();

    ~CacheClient();

    int Get(const std::string key, std::string & result, unsigned int &cost);

    int Set(const std::string key, const std::string result, unsigned int &cost);

    int BatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost);

    int BatchSet(const std::map<std::string, std::string> &SetMap, unsigned int &cost);

    int CoBatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost);

    int ThreadPoolBatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost);
 private:
    
    int GetSlotNodeMap();

    unsigned int GetSlot(const std::string key);

    int GetTargetServer(const std::string key, std::string &ip, unsigned int &port);

   //  int PipelineProcess(redisContext *conn, struct timeval access_timeout, std::vector<std::string> & pipeline_cmd, std::vector<std::string> &pipeline_resp, std::vector<bool> &pipeline_resp_status);

    int Connect(const std::string hostname, const unsigned int port, redisContext * &conn);

    int DisConnect(redisContext *conn);

    int HandelMovedCode(std::string replyStr, std::string &movedIp, unsigned int &movedPort);

    int LoadConfig();
   //  void* CoProcessCmd(std::pair<std::String, unsigned int> target,
   //    std::vector<std::String> cmds,
   //    std::map<std::string, std::string> &GetMap);


 private:

    std::map<std::string, NodeInfo> nodeInfoMap;
    std::string slotNodeBitMap[16384];
    redisContext *_conn;
    mmtraining::WorkerThreadPool _workerThreadPool;

}; 