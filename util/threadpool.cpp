
#include "threadpool.h"
#include <stdio.h>
namespace mmtraining {

/////////////////////////////////////////////////Thread

Thread::Thread() : running(false), target(NULL), tid(-1) {}

Thread::Thread(Runnable& t) : running(false), target(&t), tid(-1) {}

Thread::~Thread() {
  // TODO: 释放资源

  // delete target;
  running = false;
}

int Thread::Start() {
  // TODO: 启动线程, 运行 Run
  if (pthread_create(&tid, NULL, threadGuide, this) != 0) {
    return -1;
  }
  running = true;
  return 0;
}

pthread_t Thread::GetId() const {
  // TODO: 启动线程, 运行 Run

  return tid;
}

void* Thread::threadGuide(void* args) {
  Thread* thread = (Thread*)args;
  thread->Run();
  return thread;
}

int Thread::Run() {
  running = true;

  // 处理逻辑
  int ret = -1;
  if (target != NULL) {  // 若指定了target, 则运行target逻辑
    ret = target->Run();

    target = NULL;
  } else {  // 否则运行 DoRun 逻辑
    ret = this->DoRun();
  }

  running = false;
  return ret;
}

int Thread::DoRun() { return 0; }

bool Thread::IsRunning() const { return running; }

int Thread::Join() {
  // TODO: 完成代码
  pthread_join(tid, NULL);
  running = false;
  return 0;
}

/////////////////////////////////////////////////ThreadPool

ThreadPool::ThreadPool() {}

ThreadPool::~ThreadPool() {
  // TODO: 完成代码
  while (!threads.empty()) {
    //尾部处理
    Thread* temp = threads[threads.size() - 1];
    threads.pop_back();
    delete temp;
  }
}

int ThreadPool::Start(int threadCount, Runnable& target) {
  // TODO: 完成代码
  for (int i = 0; i < threadCount; i++) {
    Thread* thread = new Thread(target);
    threads.push_back(thread);
    thread->Start();
  }
  return 0;
}

int ThreadPool::JoinAll() {
  // TODO: 完成代码
  while (!threads.empty()) {
    //尾部处理
    threads[threads.size() - 1]->Join();
    threads.pop_back();
  }

  return 0;
}

///////////////////////////////////////////////WorkQueue

WorkQueue::WorkQueue() : shutdown(false) {
  // TODO: 初始化
  pthread_mutex_init(&mutex, NULL);
  // pthread_mutex_init( &cond_mutex, NULL );
  // pthread_cond_init( &cond, NULL );
  sem_init(&full, 0, 0);
}

WorkQueue::~WorkQueue() {
  // TODO: 释放资源
  shutdown = true;
  works.clear();
  // delete works;
  pthread_mutex_destroy(&mutex);
  sem_destroy(&full);
}

int WorkQueue::AddWork(Work* work) {
  // std::cout<<"add"<<std::endl;

  pthread_mutex_lock(&mutex);
  works.push_back(work);

  sem_post(&full);
  pthread_mutex_unlock(&mutex);
  return 0;
}

Work* WorkQueue::GetWork() {

  sem_wait(&full);
  pthread_mutex_lock(&mutex);
  Work* front = works.front();

  works.pop_front();
  pthread_mutex_unlock(&mutex);
  return front;
}

int WorkQueue::Shutdown(int thread_num) {
  // TODO: 完成代码
  if (shutdown == false) {
    shutdown = true;

    for (int i = 0; i < thread_num; i++) {
      sem_post(&full);
    }
    return 0;
  }
  return -1;
}

bool WorkQueue::IsShutdown() { return shutdown; }

/////////////////////////////////////////////////Worker

Worker::Worker(WorkQueue& queue) : workQueue(queue) {}

Worker::~Worker() {
  // TODO: 释放资源
}

int Worker::Run() {
  // TODO: 工作循环
  // std::cout<<"check"<<std::endl;
  while (!workQueue.IsShutdown()) {

    Work* work = workQueue.GetWork();
    if (work == NULL) {
      continue;
    }

    //运行逻辑
    work->DoWork();
    if (work->NeedDelete()) {
      delete work;
      work = NULL;
    }
  }
  return -1;
}

/////////////////////////////////////////////////WorkerThreadPool

WorkerThreadPool::WorkerThreadPool() : worker(workQueue) {}

WorkerThreadPool::~WorkerThreadPool() {
  // TODO: 完成代码
}

int WorkerThreadPool::Start(int threadCount) {
  thread_num = threadCount;
  return pool.Start(threadCount, worker);
}

int WorkerThreadPool::Start(int threadCount, Runnable& target) {
  return pool.Start(threadCount, target);
}

int WorkerThreadPool::AddWork(Work* work) { return workQueue.AddWork(work); }

int WorkerThreadPool::Shutdown() { return workQueue.Shutdown(thread_num); }

int WorkerThreadPool::JoinAll() { return pool.JoinAll(); }

}  // mmtraining
