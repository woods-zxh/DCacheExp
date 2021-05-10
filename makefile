
Start: Start.o cacheClient.o logicSvr.o storageClient.o threadpool.o 
	g++ -std=c++11 Start.o cacheClient.o logicSvr.o storageClient.o threadpool.o -o Start -g -L./cache/util/libco/lib  -L./cache/util/tinyxml -ltinyxml -lhiredis -lcolib -lpthread -ldl

Start.o: Start.cpp
	g++ -std=c++11 -c Start.cpp -g

cacheClient.o : cache/cacheClient.cpp cache/cacheClient.h 
	g++ -std=c++11 -c cache/cacheClient.cpp -g

logicSvr.o : logic/logicSvr.cpp logic/logicSvr.h
	g++ -std=c++11 -c logic/logicSvr.cpp -g

storageClient.o : storage/storageClient.cpp storage/storageClient.h
	g++ -std=c++11 -c storage/storageClient.cpp -g

threadpool.o: cache/util/threadpool.cpp cache/util/threadpool.h
	g++ -std=c++11 -c cache/util/threadpool.cpp -g


clean :
	-rm -f *.o Start

