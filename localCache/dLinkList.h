#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

template <typename T>
class DLinkList { 
 public:
  T *head;
  T *tail;
  int size;
  int max_size;

  DLinkList() {
    head = new T;
    head->pre = head;
    head->next = NULL;
    tail = head;
    this->size = 0;
    this->max_size = 0;
  }

  ~DLinkList() {
    if (head->next != NULL) {
      head->next->pre = NULL;
    }
    delete head;
  }

  void setMaxSize(int max_size) { this->max_size = max_size; }
  void update(T *node) {
    //新加的节点
    if (node->pre == NULL && node->next == NULL) {
      node->next = head->next;
      node->pre = head;
      if (head->next == NULL) {
        tail = node;
        node->next = NULL;
      } else {
        head->next->pre = node;
      }

      head->next = node;

      size += 1;
      if (size > max_size) eliminate();
    } else if (node->pre == head) {
      //已经是第一个
      return;
    } else if (node->next == NULL) {
      //在末尾
      tail = node->pre;
      node->pre->next = NULL;

      node->pre = head;
      node->next = head->next;
      head->next->pre = node;
      head->next = node;
    } else {
      //在中间
      node->pre->next = node->next;
      node->next->pre = node->pre;

      node->pre = head;
      node->next = head->next;
      head->next->pre = node;
      head->next = node;
    }

    // std::cout<<size<<std::endl;
  }

  void del(T *node) {

    if (node->next == NULL) {
      tail = node->pre;
      node->pre->next = NULL;
      node->pre = NULL;
    } else {
      node->pre->next = node->next;
      node->next->pre = node->pre;
      node->next = NULL;
      node->pre = NULL;
    }
    size -= 1;
  }

  //删除尾节点
  void eliminate() {
    T *node = tail->pre;
    tail->value.clear();
    tail->pre = NULL;

    tail = node;
    node->next = NULL;
    size -= 1;
  }

  T* getTail() {
    return tail;
  }
};