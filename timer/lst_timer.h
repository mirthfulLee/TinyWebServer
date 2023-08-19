#ifndef LST_TIMER
#define LST_TIMER

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../log/log.h"

class util_timer;

struct client_data {
  sockaddr_in address;
  int sockfd;
  // FIXME: 啥意思? 相互引用??
  util_timer *timer;
};

// *升序计时器链表的结点类
class util_timer {
 public:
  util_timer() : prev(NULL), next(NULL) {}

 public:
  time_t expire;

  // 一个函数指针, 任务回调函数
  // *回调函数处理的客户数据, 由定时器的执行者传递给回调函数
  void (*cb_func)(client_data *);
  client_data *user_data;
  util_timer *prev;
  util_timer *next;
};

// *升序计时器链表, 双向
class sort_timer_lst {
 public:
  sort_timer_lst();
  ~sort_timer_lst();

  // *添加计时器的接口
  void add_timer(util_timer *timer);
  void adjust_timer(util_timer *timer);
  void del_timer(util_timer *timer);
  // *SIGALRM信号每次被触发就在信号处理函数中执行依次tick函数, 从而处理到期任务
  void tick();

 private:
  // *重载的辅助函数, 将目标定时器插入到lst_head后面的位置
  void add_timer(util_timer *timer, util_timer *lst_head);

  util_timer *head;
  util_timer *tail;
};

class Utils {
 public:
  Utils() {}
  ~Utils() {}

  void init(int timeslot);

  //对文件描述符设置非阻塞
  int setnonblocking(int fd);

  //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
  // *EPOLLONESHOT避免不同线程同时处理一个连接socket上的事件
  // *类似一个互斥锁, 在处理完事件后需要立即重置这个EPOLLONESHOT事件
  void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

  //信号处理函数
  static void sig_handler(int sig);

  //设置信号函数
  void addsig(int sig, void(handler)(int), bool restart = true);

  //定时处理任务，重新定时以不断触发SIGALRM信号
  void timer_handler();

  void show_error(int connfd, const char *info);

 public:
  static int *u_pipefd;
  sort_timer_lst m_timer_lst;
  static int u_epollfd;
  int m_TIMESLOT;
};
// *定时器回调函数, 它删除非活动连接socket上的注册事件, 并关闭之
void cb_func(client_data *user_data);

#endif
