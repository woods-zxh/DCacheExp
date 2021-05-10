/* 
* 数据查找伪代码
*/
//2^32的bitmap，每个为1的值标识此位置包含node
bitmap = [0,0,0,1,0,0,1,...0]; 
//存储bitmap中获取的位置下标到cluster的映射
indexToAddr<Integer, Cluster> map; 
//key为请求中的键通过hash算法可以得到一个0-2^32-1之间的整数
n = Hash(key); 

//按照顺时针方向查找最近的虚节点
while(bitmap[n] != 1){
    //通过取模实现环
    n = (n+1)%2^32;
}

//通过在环中位置找到虚节点
VirtualNode virtualNode = GetVirtualNode(n);

//通过虚节点找到物理节点
Cluster cluster = GetCluster(virtualNode);

return cluster.addr;


<Clusters>
    <Cluster position="0"> 
        <node>
            <ip>
                182.254.228.241
            </ip>
            <port>
                7000
            </port>
        </node>
        <node>
            <ip>
                182.254.228.241
            </ip>
            <port>
                7001
            </port>
        </node>
    </Cluster>
    <Cluster position="7"> 
        <node>
            <ip>
                182.254.228.242
            </ip>
            <port>
                7000
            </port>
        </node>
    </Cluster>
    <Cluster position="15"> 
        <node>
            <ip>
                121.43.181.232
            </ip>
            <port>
                7000
            </port>
        </node>
    </Cluster>
    <Cluster position="23"> 
        <node>
            <ip>
                121.43.181.230
            </ip>
            <port>
                7000
            </port>
        </node>
    </Cluster>
</Clusters>


/**
* 批量pipeline
*/

//keys为一个string数组
keys = vector<string>;
// nodeCMD，是以节点地址为key的map,用于收集相同节点的命令
nodeCMD = map<addr, string>;
for key in keys {
    //获得slot
    slot = GetSlot(key);
    //查询路由表获得节点地址
    nodeAddr = GetNodeAddr(slot);
    //添加新的key
    nodeCMD[nodeAddr] = nodeCMD[nodeAddr] + key;
}
for addr in nodeCMD.keys {
    //发送查询命令
    SendCmd(addr, nodeCMD[addr]);
}
//组合结果
CombineCmd();

/**
* 协程实现伪代码
*/

// nodeCMD，是以节点地址为key的map,用于收集相同节点的命令
nodeCMD = map<addr, string>;
// 获取相同节点命令
for key in keys {
    //获得slot
    slot = GetSlot(key);
    //查询路由表获得节点地址
    nodeAddr = GetNodeAddr(slot);
    //添加新的key
    nodeCMD[nodeAddr] = nodeCMD[nodeAddr] + key;
}

//使用协程间公共map保存结果
resultMap = map<string,string>
for addr in nodeCMD.keys {
    //为每一个节点创建一个协程
    co_create(addr, nodeCMD[addr],resultMap);
    //让出刚创建的协程给主调度协程
    co_resume();
}
