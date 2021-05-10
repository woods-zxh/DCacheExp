#include <iostream>
#include "localCache.h"
using namespace std;

int main() {
    int ret, ver;
    string key, value;
    LocalCache localCache(5);


    ret = localCache.Set("1", "1", ver);
    cout << ret << endl;

    ret = localCache.Get("1", value, ver);
    cout << ret << " " << value << " " << ver << endl;

    ret = localCache.Set("1", "2", ver);
    cout << ret << " " << ver << endl;

}