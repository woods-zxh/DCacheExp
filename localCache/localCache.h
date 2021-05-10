#pragma once

#include <string>
#include <map>

#include "dLinkList.h"

struct Node
{
    struct Node* pre;
    struct Node* next;
    std::string key;
    std::string value;
    int ver;

};


class LocalCache {
 public:

    LocalCache(int maxSize);

    ~LocalCache();
    
    //启动函数
    int Get(const std::string key, std::string &value, int &ver);

    int Set(const std::string key, const std::string value, int &ver);

    int Del(const std::string key, int ver);

 private:   


 private:
   std::hash_map<std::string, Node> storeMap;
   DLinkList<Node> linkList;
   int _maxSize;
};