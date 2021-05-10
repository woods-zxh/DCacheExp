#include <iostream>

#include "localCache.h"

LocalCache::LocalCache(int maxSize) {

    linkList.setMaxSize(maxSize);
    _maxSize =  maxSize;
}

LocalCache::~LocalCache() {}

int LocalCache::Get(const std::string key, std::string &value, int &ver) {
    std::map<std::string, Node>::iterator iter;
    iter = storeMap.find(key);

    if (iter != storeMap.end()) {
        value = iter->second.value;
        ver = iter->second.ver;
        linkList.update(&(iter->second));
        return 0;
    } else {
        return -1;
    }
}

int LocalCache::Set(const std::string key, const std::string value, int &ver) {
    //get
    int storeVer;
    int ret;
    std::map<std::string, Node>::iterator iter;
    iter = storeMap.find(key);
    if (iter != storeMap.end()) {
    //存在
        storeVer = iter->second.ver;
        if (storeVer != ver)
            return 1;
        iter->second.ver = storeVer + 1;
        ver ++;
        // std::cout << iter->second.ver << std::endl;
        linkList.update(&(iter->second));
        return 0;
    } else {
    //不存在
        if (storeMap.size() >= _maxSize) {
            std::string delKey = linkList.getTail()->key;
            storeMap.erase(delKey);
        }

        Node node;
        node.key = key;
        node.value = value;
        node.ver = 0;
        node.pre = NULL;
        node.next = NULL;
        storeMap[node.key] = node;
        linkList.update(&storeMap[node.key]);
        ver = 0;
        return 0;
    }
}


int LocalCache::Del(const std::string key, int ver) {
    //get
    int storeVer;
    int ret;

    std::map<std::string, Node>::iterator iter;
    iter = storeMap.find(key);
    storeVer = iter->second.ver;
    if (iter == storeMap.end()) 
        return -1;
    
    if (storeVer != ver) 
        return 1;

    if (iter != storeMap.end()) {
        linkList.del(&iter->second);
        storeMap.erase(key);
    }

    return 0;
}


