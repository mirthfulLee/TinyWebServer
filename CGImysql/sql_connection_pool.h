#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <error.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <list>
#include <string>

#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class connection_pool {
 public:
  MYSQL *GetConnection();               //获取数据库连接
  bool ReleaseConnection(MYSQL *conn);  //释放连接
  int GetFreeConn();                    //获取连接
  void DestroyPool();                   //销毁所有连接

  //单例模式
  static connection_pool *GetInstance();

  void init(string url, string User, string PassWord, string DataBaseName,
            int Port, int MaxConn, int close_log);

 private:
  connection_pool();
  ~connection_pool();

  int m_MaxConn;   //最大连接数
  int m_CurConn;   //当前已使用的连接数
  int m_FreeConn;  //当前空闲的连接数
  locker lock;
  list<MYSQL *> connList;  //连接池
  sem reserve;

 public:
  string m_url;           //主机地址
  string m_Port;          //数据库端口号
  string m_User;          //登陆数据库用户名
  string m_PassWord;      //登陆数据库密码
  string m_DatabaseName;  //使用数据库名
  int m_close_log;        //日志开关
};

// ?这个类是干嘛的?
// *RAII: Resource Acquisition Is Initialization-资源获取即初始化
// *使用RAII即可让操作系统根据局部变量的生命周期自动销毁资源, 而不再需要手动销毁
class connectionRAII {
 public:
	// *构造函数实现资源(MySQL连接)的获取
  connectionRAII(MYSQL **con, connection_pool *connPool);
	// *当局部变量生命周期结束时执行, 释放资源(MySQL连接)
  ~connectionRAII();

 private:
  MYSQL *conRAII;
  connection_pool *poolRAII;
};

#endif
