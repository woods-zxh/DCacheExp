#include <iostream>

#include "storageClient.h"

StorageClient::StorageClient() {};

StorageClient::~StorageClient() {};

int StorageClient::Get(const std::string key, std::string & result, unsigned int &cost) {
    //TODO 
    std::cout << "in storage get" << std::endl;
    return -1;
}

int StorageClient::Set(const std::string key, const std::string result, unsigned int &cost) {
    //TODO
    std::cout << "in storage set" << std::endl;
    return -1;
}