#include "socket.h"
#include "threadpool.h"
#include <memory>
#include <iostream>
#include <stdio.h>
#include <errno.h>
using namespace mmtraining;
// g++ -g echosvr-thread.cpp socket.cpp threadpool.cpp -o echosvr-thread
// -lpthread

//对请求的逻辑
class RequestWork : public Work {
 public:
  RequestWork(ClientSocket* client_socket) : client_socket(client_socket) {}

  virtual ~RequestWork() { std::cout << "work over " << std::endl; }

  virtual bool NeedDelete() const { return false; }

  virtual int DoWork() {
    std::cout << "do work:write back " << std::endl;
    int len;
    char buf[LINE_BUFFER];

    len = client_socket->Read(buf, LINE_BUFFER);
    if (len > 0) {
      std::cout << buf << std::endl;
      client_socket->Write(buf, len);
    }
    client_socket->Close();

    return 0;
  }

 private:
  ClientSocket* client_socket;
};

class Server : public Thread {
 public:
  int Start(unsigned short port) {
    if (serverSocket.Listen("127.0.0.1", port) != 0) {
      std::cout << "Test ERROR: Listen error, " << strerror(errno) << std::endl;
      return -1;
    }
    return Thread::Start();
  }

  ClientSocket* Accept() { return serverSocket.Accept(); }

 private:
  int DoRun() { return 0; }

  ServerSocket serverSocket;
};

int main(int argc, char** argv) {
  int thread_num = 1;

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  unsigned short port = (unsigned short)atoi(argv[1]);

  Server server;
  if (server.Start(port) != 0) {
    return 1;
  }

  WorkerThreadPool workerThreadPool = WorkerThreadPool();

  workerThreadPool.Start(thread_num);

  while (true) {
    ClientSocket* client = server.Accept();
    if (client == NULL) {
      std::cout << "Test ERROR: Accept error, " << strerror(errno) << std::endl;
      return -1;
    } else {
      RequestWork* work = new RequestWork(client);
      workerThreadPool.AddWork(work);
    }
  }

  server.Join();

  return 0;
}
