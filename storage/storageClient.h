#pragma once
#include <string>

class StorageClient {
 public:

    StorageClient();

    ~StorageClient();

    int Get(const std::string key, std::string & result, unsigned int &cost);

    int Set(const std::string key, const std::string result, unsigned int &cost);

 private:
    

};