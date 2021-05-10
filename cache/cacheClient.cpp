#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <string.h>


#include "cacheClient.h"
#include "hiredis.h"
#include "util/libco/co_routine.h"
#include "util/crc16.h"
#include "util/tinyxml/tinyxml.h"

CacheClient::CacheClient() {
    GetSlotNodeMap();

    //开启多个线程
    // _workerThreadPool = mmtraining::WorkerThreadPool();

    _workerThreadPool.Start(5);

    LoadConfig();
};

CacheClient::~CacheClient() {};

int CacheClient::LoadConfig() {
    TiXmlDocument doc;
    if(!doc.LoadFile("cache/config/clusters.xml"))
    {
        std::cerr << doc.ErrorDesc() << std::endl;
        return -1;
    }

    TiXmlElement* root = doc.FirstChildElement();
    if(root == NULL)
    {
        std::cerr << "Failed to load file: No root element." << std::endl;
        doc.Clear();
        return -1;
    }

    for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
    {
        std::string elemName = elem->Value();
        const char* attr;
        attr = elem->Attribute("cid");
        if(strcmp(attr,"0")==0)
        {
            TiXmlElement* e1 = elem->FirstChildElement("node");
            TiXmlElement* e2= e1->FirstChildElement("ip");
            TiXmlNode* e3=e2->FirstChild();
            std::cout <<"cid=0" << e3->ToText()->Value() << std::endl;

        }
        else if(strcmp(attr,"1")==0)
        {
            TiXmlElement* e1 = elem->FirstChildElement("node");
            TiXmlElement* e2= e1->FirstChildElement("ip");
            TiXmlNode* e3=e2->FirstChild();
            std::cout <<"cid=1" << e3->ToText()->Value() << std::endl;
        }
    }
    doc.Clear();
    return 0;
}

int CacheClient::GetSlotNodeMap() {
    std::string ip = "121.43.181.232";
    unsigned int port = 7000;

    redisContext *conn;  
    Connect(ip, port, conn);

    redisReply *reply;
    reply = (redisReply *)redisCommand(conn,"CLUSTER NODES");
    // std::cout << "in GetSlotNodeMap " << std::endl;

    //截取行
    std::vector<char*> nodeInfoLine;
    char *p;
    const char *d = "\n";
    p = strtok(reply->str,d);
    while(p) {
        nodeInfoLine.push_back(p);
        // printf("%s\n",p);
        p = strtok(NULL,d);
    }

    //截取每行的空格
    // int nodeId = 1;
    for(char* line : nodeInfoLine) {
            NodeInfo nodeInfo;
            int index = 1;
            char *p;
            const char *d = " ";
            p = strtok(line,d);
            while(p) {
                if (index == 1) {
                    nodeInfo.hashValue = p;
                    // printf("hashValue %s\n",p);
                } else if (index == 2) {
                    std::string addr = p;
                    int mou = addr.find(":");
                    nodeInfo.ip = addr.substr(0, mou);
                    nodeInfo.port = std::stoi(addr.substr(mou+1));
                    // printf("%s %d\n",nodeInfo.ip.c_str(), nodeInfo.port);
                } else if (index == 3) {
                    if(p[0] != 'm' && p[1] != 'a') {
                        nodeInfo.isMaster = false;
                    } else {
                        nodeInfo.isMaster = true;
                    }
                    // printf("ms %s\n",p);
                } else if (index == 4) {
                    // printf("master %s\n",p);
                    nodeInfo.masterHashValue = p;
                } else if (index == 9) {
                    int slotStart = 0;
                    int slotEnd = 0;
                    std::string slotRange = p;
                    int mou = slotRange.find("-");
                    slotStart = std::stoi(slotRange.substr(0, mou+1));
                    slotEnd = std::stoi(slotRange.substr(mou+1));

                    //填写slotbitmap
                    if (nodeInfo.isMaster) {
                        for(int i = slotStart; i < slotEnd; i++) {
                            this->slotNodeBitMap[i] = nodeInfo.hashValue;
                        }
                    }

                    // printf("%d %d\n",slotStart, slotEnd);
                }
                // printf("%d\n",index);
                p=strtok(NULL,d);
                index++;
            }
            this->nodeInfoMap[nodeInfo.hashValue] = nodeInfo;
            // nodeId++;
    }
}

int CacheClient::Connect(const std::string hostname, const unsigned int port, redisContext * &conn) {
    struct timeval timeout = { 1, 500000 };
    conn = redisConnectWithTimeout((char*)hostname.c_str(), port, timeout);

    if (conn == NULL || conn->err) {
		if (conn) {
             printf("Connection error: %s\n", conn->errstr);
             redisFree(conn);
		} else {
             printf("Connection error: can't allocate redis context\n");
		}
        return -1;
    }

    return 0;
}

int CacheClient::DisConnect(redisContext *conn) {
    /* Disconnects and frees the context */
     redisFree(conn);
     return 0;
}

int CacheClient::HandelMovedCode(std::string replyStr, std::string &movedIp, unsigned int &movedPort) {
    int ret = 0;

    int pos = replyStr.rfind(" ");

    std::string addrStr = replyStr.substr(pos);

    pos = addrStr.rfind(":");

    movedIp = addrStr.substr(0, pos);
    movedPort = std::stoi(addrStr.substr(pos + 1));

    //获得slot值
    int slot = 0;

    //更新slot表
    for(auto &node : this->nodeInfoMap) {
        if (node.second.ip == movedIp && node.second.port == movedPort) { 
            this->slotNodeBitMap[slot] = node.second.hashValue;
        }
    }
    std::cout << "int moved" << movedIp << " " << movedPort << std::endl;
    return ret;
}

int CacheClient::Get(const std::string key, std::string & result, unsigned int &cost) {
    //TODO 
    // std::cout << "in cache get" << std::endl;
    /* Get */
    int ret = 0;
    std::string ip;
    unsigned int port;
    GetTargetServer(key, ip, port);

    redisContext *conn;  
    Connect(ip, port, conn);

    redisReply *reply;
    reply = (redisReply *)redisCommand(conn,"GET %s", key.c_str());

    while(reply->type == REDIS_REPLY_ERROR) {
        std::string movedIp = ip;
        unsigned int movedPort = port;
        ret = HandelMovedCode(reply->str, movedIp, movedPort);

        Connect(movedIp, movedPort, conn);
        reply = (redisReply *)redisCommand(conn,"GET %s", key.c_str());
        std::cout << ip << " " << movedPort << std::endl;
    }

    printf("GET reply: %s reply type %d\n", reply->str, reply->type);
    result = reply->str;

    freeReplyObject(reply);
    DisConnect(conn);
    return ret;
}

int CacheClient::Set(const std::string key, const std::string result, unsigned int &cost) {
    //TODO
    // std::cout << "in cache set" << std::endl;
    /* Set */
    std::string ip;
    unsigned int port;
    GetTargetServer(key, ip, port);

    redisContext *conn;  
    Connect(ip, port, conn);

    redisReply *reply;
    reply = (redisReply *)redisCommand(conn,"SET %s %s", key.c_str(), result.c_str());
    printf("SET: %s\n", reply->str);

    freeReplyObject(reply);
    DisConnect(conn);
    return -1; 
}

//using crc16 & 16383 to get slot
//暂确定三个节点7000：0-5000；7001：5001-10000；7002 10001-16383
unsigned int CacheClient::GetSlot(const std::string key) {

    return crc16(key.c_str(), key.length()) & 16383;
}

int CacheClient::GetTargetServer(const std::string key, std::string &ip, unsigned int &port) {
    unsigned int slot = GetSlot(key);
    //找到node_id
    std::string nodeId = this->slotNodeBitMap[slot];
    
    //根据node_id找到ip地址，注意主从交换
    if ((this->nodeInfoMap.find(nodeId) == this->nodeInfoMap.end()))
        return -1;

    NodeInfo nodeInfo = this->nodeInfoMap[nodeId];
    if (!nodeInfo.isMaster) 
        nodeInfo = this->nodeInfoMap[nodeInfo.masterHashValue];
    ip = nodeInfo.ip;
    port = nodeInfo.port;

    return 0;
}

int PipelineProcess(redisContext *conn, struct timeval access_timeout, std::vector<std::string> & pipeline_cmd, std::vector<std::string> &pipeline_resp, std::vector<bool> &pipeline_resp_status){
    if (0 == conn) {return -1;}
    redisSetTimeout(conn, access_timeout);
    // printf("2.0\n");
    for (int i = 0; i < pipeline_cmd.size(); i++)
    {
        // printf("%d, %d, %s\n", i, pipeline_cmd.size(), pipeline_cmd[i].c_str());
        redisAppendCommand(conn, pipeline_cmd[i].c_str());
    }
    // printf("2.1\n");
    for (int i = 0; i < pipeline_cmd.size();i++)
    {
        bool status = false;
        std::string resp_str = "";
        redisReply *reply = 0;

        if(redisGetReply(conn, (void **)&reply) == REDIS_OK
                && reply != NULL
                && reply->type == REDIS_REPLY_STRING)
        {
            status = true;
            resp_str = reply->str;
        }
        //free
        freeReplyObject(reply);

        pipeline_resp_status.push_back(status);
        pipeline_resp.push_back(resp_str);

    }
    return 0;
}

int CacheClient::BatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost) {
    // printf("1\n");
    int ret = 0;
    //根据目标服务器分组
    std::map<std::pair<std::string, unsigned int>,std::vector<std::string>> groups;
    for (int i = 0; i < keyList.size(); i++) {
        std::string ip;
        unsigned int port;
        GetTargetServer(keyList[i], ip, port);

        groups[std::make_pair(ip, port)].push_back(keyList[i]);
        GetMap[keyList[i]] = "";

        // std::cout << keyList[i] << ip << " "<< port << std::endl;
    }
    // printf("2\n");
    //顺序常规构建操作命令
    std::map<std::pair<std::string, unsigned int>, std::vector<std::string>>::iterator iter;
    
    for (iter = groups.begin(); iter != groups.end(); iter++) {
        std::vector<std::string> pipelineCmd;

        redisContext *conn;  
        ret = Connect(iter->first.first, iter->first.second, conn);
        if (ret != 0) {
            printf("connect %s %u fail", iter->first.first.c_str(), iter->first.second);
            continue;
        }
        printf("connect %s %u succ\n", iter->first.first.c_str(), iter->first.second);
        // printf("3\n");
        for (std::string key : iter->second) {
            std::string cmd;
            cmd = "GET " + key;
            pipelineCmd.push_back(cmd);
        }

        struct timeval accessTimeout = { 1, 500000 };
        std::vector<std::string> pipelineResp; 
        std::vector<bool> pipelineRespStatus;
        PipelineProcess(conn, accessTimeout, pipelineCmd, pipelineResp, pipelineRespStatus);
        DisConnect(conn);
        for (int i = 0; i < pipelineResp.size(); i++) {
            GetMap[iter->second[i]] = pipelineResp[i];
        }
    }
    return 0;
}

int CacheClient::BatchSet(const std::map<std::string, std::string> &SetMap, unsigned int &cost) {
    // int ret = 0;
    // //根据目标服务器分组
    // std::map<std::pair<std::string, unsigned int>,std::vector<std::string>> groups;
    // for (int i = 0; i < keyList.size(); i++) {
    //     std::string ip;
    //     unsigned int port;
    //     GetTargetServer(keyList[i], ip, port);

    //     groups[std::make_pair(ip, port)].push_back(keyList[i]);
    //     GetMap[keyList[i]] = "";

    //     // std::cout << keyList[i] << ip << " "<< port << std::endl;
    // }
    return 0;
}

//单个协程处理函数
struct stEnv_t
{
	std::string ip;
    unsigned int port;
	std::vector<std::string> *cmds;
    std::map<std::string, std::string> *GetMapPtr;
};

void* CoProcessCmd(void* args) {
    co_enable_hook_sys();

    stEnv_t* env = (stEnv_t*)args;
    
    std::vector<std::string> pipelineCmd;
    redisContext *conn;  
    struct timeval timeout = { 1, 500000 };
    conn = redisConnectWithTimeout(env->ip.c_str(), env->port, timeout);

    printf("connect %s %u succ\n", env->ip.c_str(), env->port);
    for (std::string key : *(env->cmds)) {
        std::string cmd;
        cmd = "GET " + key;
        pipelineCmd.push_back(cmd);
    }

    std::vector<std::string> pipelineResp; 
    std::vector<bool> pipelineRespStatus;

    PipelineProcess(conn, timeout, pipelineCmd, pipelineResp, pipelineRespStatus);

    redisFree(conn);
    for (int i = 0; i < pipelineResp.size(); i++) {
        std::string key = (*(env->cmds))[i];
        (*(env->GetMapPtr))[key] = pipelineResp[i];
        printf("%s, %s\n", key.c_str(), pipelineResp[i].c_str());
    }
    return NULL;
}

//检查是否都已经获得答案
struct stFEnv_t
{
    std::map<std::string, std::string> *GetMapPtr;
};

int checkCmdResult(void* args) {
    stFEnv_t* fEnv = (stFEnv_t*)args;
    
    std::map<std::string, std::string>::iterator iter;
    for (iter = (*fEnv->GetMapPtr).begin(); iter != (*fEnv->GetMapPtr).end(); iter++) {
        if (iter->second == "") {
            return 0;
        }
    }
    return -1;
}

//协程版本batchget
int CacheClient::CoBatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost) {

    int ret = 0;
    //根据目标服务器分组
    std::map<std::pair<std::string, unsigned int>, std::vector<std::string>> groups;
    for (int i = 0; i < keyList.size(); i++) {
        std::string ip;
        unsigned int port;
        GetTargetServer(keyList[i], ip, port);

        groups[std::make_pair(ip, port)].push_back(keyList[i]);
        GetMap[keyList[i]] = "";

        // std::cout << keyList[i] << ip << " "<< port << std::endl;
    }
    //使用协程发出请求  

    std::map<std::pair<std::string, unsigned int>, std::vector<std::string>>::iterator iter;

    for (iter = groups.begin(); iter != groups.end(); iter++) {
        stEnv_t* env = new stEnv_t;
        env->ip = iter->first.first;
        env->port = iter->first.second;
        env->cmds = &(iter->second);
        env->GetMapPtr = &GetMap;
        stCoRoutine_t* routine;
        co_create(&routine, NULL, CoProcessCmd, env);
        co_resume(routine);
    }   

    stFEnv_t* fEnv = new stFEnv_t;
    fEnv->GetMapPtr = &GetMap;
    co_eventloop(co_get_epoll_ct(), checkCmdResult, fEnv);
    return 0;
}

//定义线程池的任务
class SendCmdWork : public mmtraining::Work {
 public:
  SendCmdWork(std::map<std::string, std::string>* getMapPtr, 
        const std::string ip, 
        unsigned port, 
        std::vector<std::string>* cmds) : _getMapPtr(getMapPtr),_ip(ip), _port(port), _cmds(cmds){}

  virtual ~SendCmdWork() { std::cout << "work over " << std::endl; }

  virtual bool NeedDelete() const { return true; }

  virtual int DoWork() {
    std::cout << _ip << ":" << _port << " do work:write back " << std::endl;

    std::vector<std::string> pipelineCmd;
    redisContext *conn;  
    struct timeval timeout = { 1, 500000 };
    conn = redisConnectWithTimeout(_ip.c_str(), _port, timeout);

    // printf("connect %s %u succ\n", _ip.c_str(), _port);
    for (std::string key : *(_cmds)) {
        std::string cmd;
        cmd = "GET " + key;
        pipelineCmd.push_back(cmd);
    }

    std::vector<std::string> pipelineResp; 
    std::vector<bool> pipelineRespStatus;

    PipelineProcess(conn, timeout, pipelineCmd, pipelineResp, pipelineRespStatus);

    redisFree(conn);
    for (int i = 0; i < pipelineResp.size(); i++) {
        std::string key = (*(_cmds))[i];
        (*(_getMapPtr))[key] = pipelineResp[i];
        // printf("%s, %s\n", key.c_str(), pipelineResp[i].c_str());
    }
    return 0;
  }

 private:
  std::map<std::string, std::string>* _getMapPtr;
  std::string _ip;
  unsigned int _port;
  std::vector<std::string> *_cmds;
};

bool checkMapResult(std::map<std::string, std::string> &GetMap) {
    std::map<std::string, std::string>::iterator iter;
    for (iter = GetMap.begin(); iter != GetMap.end(); iter++) {
        if (iter->second == "") {
            return false;
        }
    }
    return true;
}

//线程池版本batchget
int CacheClient::ThreadPoolBatchGet(const std::vector<std::string> &keyList, std::map<std::string, std::string> &GetMap, unsigned int &cost) {
    int ret = 0;
    //根据目标服务器分组
    std::map<std::pair<std::string, unsigned int>, std::vector<std::string>> groups;
    for (int i = 0; i < keyList.size(); i++) {
        std::string ip;
        unsigned int port;
        GetTargetServer(keyList[i], ip, port);

        groups[std::make_pair(ip, port)].push_back(keyList[i]);
        GetMap[keyList[i]] = "";

        // std::cout << keyList[i] << ip << " "<< port << std::endl;
    }
    //使用协程发出请求  

    std::map<std::pair<std::string, unsigned int>, std::vector<std::string>>::iterator iter;

    for (iter = groups.begin(); iter != groups.end(); iter++) {
        std::cout  << iter->first.first << " "<< iter->first.second << std::endl;
        SendCmdWork* work = new SendCmdWork(&GetMap, iter->first.first, iter->first.second, &(iter->second));
        _workerThreadPool.AddWork(work);

    }   

    //持续检查GetMap是否被填满
    while (!checkMapResult(GetMap)){}
    
    return ret;
}