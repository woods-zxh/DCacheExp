
#include "threadpool.h"
#include <iostream>
#include <strings.h>
using namespace mmtraining;
// g++ -g threadpool.cpp testthreadpool.cpp -o testthreadpool -lpthread
/**
 * 测试用 Work
 */
class TestWork : public Work {
 public:
  TestWork(int v) : value(v) {}

  virtual ~TestWork() {}

  virtual bool NeedDelete() const { return false; }

  virtual int DoWork() {
    int oldValue = value;
    value = -value;
    std::cout << "\tdo work " << oldValue << " -> " << value << std::endl;
    return 0;
  }

  int GetValue() const { return value; }

 private:
  int value;
};

/**
 * 往队列加入两个工作, 测试是否可以按序取出
 */
int TestWorkQueue() {
  WorkQueue queue;

  // Test IsShutdown
  if (queue.IsShutdown()) {
    std::cout << "TestWorkQueue ERROR: Queue shutted down" << std::endl;
    return -1;
  }

  // Test AddWork
  TestWork firstWork(123);
  if (queue.AddWork(&firstWork) != 0) {
    std::cout << "TestWorkQueue ERROR: AddWork error" << std::endl;
    return -1;
  }

  TestWork secondWork(456);
  if (queue.AddWork(&secondWork) != 0) {
    std::cout << "TestWorkQueue ERROR: AddWork error" << std::endl;
    return -1;
  }

  // Test GetWork
  TestWork* w1 = dynamic_cast<TestWork*>(queue.GetWork());
  if (w1 != &firstWork) {
    std::cout << "TestWorkQueue ERROR: GetWork error" << std::endl;
    return -1;
  } else if (w1->GetValue() != 123) {
    std::cout << "TestWorkQueue ERROR: TestWorkQueue error" << std::endl;
    return -1;
  }

  TestWork* w2 = dynamic_cast<TestWork*>(queue.GetWork());
  if (w2 != &secondWork) {
    std::cout << "TestWorkQueue ERROR: GetWork error" << std::endl;
    return -1;
  } else if (w2->GetValue() != 456) {
    std::cout << "TestWorkQueue ERROR: TestWorkQueue error" << std::endl;
    return -1;
  }

  // Test Shutdown
  if (queue.Shutdown(1) != 0) {
    std::cout << "TestWorkQueue ERROR: shutdown error" << std::endl;
    return -1;
  }

  if (!queue.IsShutdown()) {
    std::cout << "TestWorkQueue ERROR: Shutdown error" << std::endl;
    return -1;
  }

  std::cout << "TestWorkQueue OK" << std::endl;
  return 0;
}

/**
 * 测试是否可以通过队列驱动工作线程工作与退出
 */
int TestWorkerThread() {
  WorkQueue queue;
  Worker worker(queue);

  Thread thread(worker);

  // Test IsRunning
  if (thread.IsRunning()) {
    std::cout << "TestWorkerThread ERROR: worker already running" << std::endl;
    return -1;
  }

  // Test Start
  if (thread.Start() != 0) {
    std::cout << "TestWorkerThread ERROR: Start worker error" << std::endl;
    return -1;
  }

  if (!thread.IsRunning()) {
    std::cout << "TestWorkerThread ERROR: worker not running" << std::endl;
    return -1;
  }

  // Test AddWork
  TestWork work(123);
  if (queue.AddWork(&work) != 0) {
    std::cout << "TestWorkerThread ERROR: AddWork error" << std::endl;
    return -1;
  }

  // thread switch
  while (work.GetValue() != -123) {
  }
  // pthread_join(thread.GetId(),NULL);
  // sleep(2);
  // Test Value
  if (work.GetValue() != -123) {
    std::cout << "TestWorkerThread ERROR: value error" << std::endl;
    return -1;
    // while(true){
    //     // cout<<"pass"<<endl;
    //     pthread_yield();
    // }
  }
  pthread_yield();
  // stop thread
  if (queue.Shutdown(1) != 0) {
    std::cout << "TestWorkerThread ERROR: shutdown error" << std::endl;
    return -1;
  }

  // thread switch
  pthread_yield();
  pthread_join(thread.GetId(), NULL);
  if (thread.IsRunning()) {
    std::cout << "TestWorkerThread ERROR: worker still running" << std::endl;
    return -1;
  }

  thread.Join();

  return 0;
}

/**
 * 测试是否可以通过队列驱动工作线程池工作与退出
 */
int TestWorkerThreadPool() {
  WorkerThreadPool pool;

  // Test Start
  if (pool.Start(3) != 0) {
    std::cout << "TestWorkerThreadPool ERROR: start error" << std::endl;
    return -1;
  }

  // AddWork
  TestWork work(123);
  if (pool.AddWork(&work) != 0) {
    std::cout << "TestWorkerThreadPool ERROR: start error" << std::endl;
    return -1;
  }
  TestWork work1(234);
  pool.AddWork(&work1);
  // thread switch
  pthread_yield();
  // Test Value
  if (work.GetValue() != -123) {
    std::cout << "TestWorkerThreadPool ERROR: value error" << work.GetValue()
              << std::endl;
    return -1;
  }

  if (pool.Shutdown() != 0) {
    std::cout << "TestWorkerThreadPool ERROR: shutdown error" << std::endl;
    return -1;
  }

  pool.JoinAll();

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " -f [queue|thread|pool]" << std::endl;
    return 1;
  }

  if (strcasecmp(argv[2], "queue") == 0) {
    return TestWorkQueue();
  } else if (strcasecmp(argv[2], "thread") == 0) {
    return TestWorkerThread();
  } else if (strcasecmp(argv[2], "pool") == 0) {
    return TestWorkerThreadPool();
  } else {
    return 2;
  }
}
