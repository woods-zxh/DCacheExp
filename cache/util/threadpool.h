
#ifndef MM_TRANING_THREAD_POOL_H
#define MM_TRANING_THREAD_POOL_H

#include <pthread.h>
#include <deque>
#include <vector>
#include "runnable.h"
#include <iostream>
#include <semaphore.h>

namespace mmtraining {

/**
 * 线程
 */
class Thread : public Runnable {
 public:
  /**
   * 构造函数
   */
  Thread();

  /**
   * 构造函数
   */
  Thread(Runnable& t);

  /**
   * 析构函数
   */
  virtual ~Thread();

  /**
   * 启动线程
   * @return 0 成功, -1 失败
   */
  int Start();

  /**
   * 获取线程id
   */
  pthread_t GetId() const;

  /**
   * 线程处理
   */
  int Run();

  /**
  * 线程静态函数
  */
  static void* threadGuide(void* arg);

  /**
   * 获取线程是否正在运行
   */
  bool IsRunning() const;

  /**
   * 等待线程退出，并回收线程资源
   * @return 0 成功, -1 失败
   */
  int Join();

 protected:
  /**
   * 线程处理
   */
  virtual int DoRun();

  bool running;
  Runnable* target;
  pthread_t tid;
};

/**
 * 线程池, 维护了一组线程
 */
class ThreadPool {
 public:
  /**
   * 构造函数
   */
  ThreadPool();

  /**
   * 析构函数
   */
  virtual ~ThreadPool();

  /**
   * 启动线程, 运行目标逻辑
   * @param threadCount 启动的线程数
   * @param target 目标运行逻辑
   * @return 0 成功, -1 失败
   */
  virtual int Start(int threadCount, Runnable& target);

  /**
   * 等待所有线程退出，并回收线程资源
   * @return 0 成功, -1 失败
   */
  int JoinAll();

 protected:
  typedef std::vector<Thread*> ThreadVec;

  ThreadVec threads;
};

/**
 * 工作基类
 */
class Work {
 public:
  /**
   * 析构函数，释放资源
   */
  virtual ~Work() {}

  /**
   * 是否需要被 delete
   */
  virtual bool NeedDelete() const = 0;

  /**
   * 工作处理逻辑
   * @return 0 处理成功, -1 处理失败
   */
  virtual int DoWork() = 0;

  // virtual int DoWork() = 0;
};

/**
 * 线程安全的FIFO工作队列
 */
class WorkQueue {
 public:
  /**
   * 构造函数, 初始化工作队列
   */
  WorkQueue();

  /**
   * 析构函数, 释放资源
   */
  ~WorkQueue();

  /**
   * 将待处理工作加入队列, 并唤醒一个线程处理
   * @param work 待处理工作
   * @return 0 成功, -1 失败
   */
  int AddWork(Work* work);

  /**
   * 从队列中取出一个工作,
   * 若队列为空, 则等待被唤醒
   * @return 工作
   */
  Work* GetWork();

  /**
   * 关闭队列, 唤醒所有工作线程
   * @return 0 成功, -1 失败
   */
  int Shutdown(int thread_num);

  /**
   * 是否被关闭
   */
  bool IsShutdown();

 private:
  typedef std::deque<Work*> Queue;
  pthread_mutex_t mutex;
  // pthread_mutex_t cond_mutex;
  // pthread_cond_t cond;
  sem_t full;
  bool shutdown;
  Queue works;
};

/**
 * 工作处理逻辑, 不断从工作队列中获取工作并处理
 */
class Worker : public Runnable {
 public:
  /**
   * 构造函数
   * @param queue 工作队列
   */
  Worker(WorkQueue& queue);

  /**
   * 析构函数
   */
  ~Worker();

  /**
* 处理逻辑, 从工作队列中取出工作并处理
*/
  int Run();

 private:
  WorkQueue& workQueue;
};

/**
 * 工作线程池, 维护了一个工作队列和一组工作线程,
 * 通过往工作队列放入数据来驱动工作线程工作
 */
class WorkerThreadPool {
 public:
  /**
   * 构造函数
   */
  WorkerThreadPool();

  /**
   * 析构函数
   */
  ~WorkerThreadPool();

  /**
   * 启动线程
   * @param threadCount 启动的线程数
   * @return 0 成功, -1 失败
   */
  int Start(int threadCount);

  /**
   * 添加工作，唤醒工作线程处理
   * @param work 工作
   * return 0 成功, -1 失败
   */
  int AddWork(Work* work);

  /**
   * 停止所有线程
   */
  int Shutdown();

  /**
   * 等待所有线程退出，并回收线程资源
   * @return 0 成功, -1 失败
   */
  int JoinAll();

 protected:
  /**
   * 启动线程
   * @param threadCount 启动的线程数
   * @return 0 成功, -1 失败
   */
  int Start(int threadCount, Runnable& target);

  WorkQueue workQueue;
  Worker worker;
  ThreadPool pool;
  int thread_num;
};

}  // mmtraining

#endif  // MM_TRANNING_THREAD_POOL_H
