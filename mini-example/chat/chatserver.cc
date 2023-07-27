

#include "codec.h"

#include "EventLoop.h"
#include "TcpServer.h"
#include "ThreadLocalSingleton.h"
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <functional>

using namespace muduo;

class ChatServer : noncopyable
{
 public:
  ChatServer(EventLoop* loop,
             const InetAddress& listenAddr)
  : server_(loop, listenAddr, "ChatServer")
  {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start()
  {
    server_.setThreadInitCallback(std::bind(&ChatServer::threadInit, this, std::placeholders::_1));
    server_.start();
  }

 private:



  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time){

    std::string message = buf->retrieveAllAsString();

    EventLoop::Functor f = std::bind(&ChatServer::distributeMessage, this,message);
    
    lock_guard<mutex> l(mutex_); 
    for (std::set<EventLoop*>::iterator it = loops_.begin();
        it != loops_.end();
        ++it)
    {
      (*it)->queueInLoop(f);
    }
  }
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG(LoggerLevel::INFO,"%s -> %s ",conn->peerAddress().toIpPort().c_str(),conn->localAddress().toIpPort().c_str());

    if (conn->connected())
    {
      LocalConnections::instance().insert(conn);
    }
    else
    {
      LocalConnections::instance().erase(conn);
    }
  }



  void distributeMessage(const string& message)
  {
    
    for (ConnectionList::iterator it = LocalConnections::instance().begin();
        it != LocalConnections::instance().end();
        ++it)
    {
      (*it).get()->send(message);
    }
  }

  void threadInit(EventLoop* loop)
  {
    LocalConnections::instance();

    lock_guard<mutex> lock(mutex_);
    loops_.insert(loop);
  }

  TcpServer server_;
  typedef std::set<TcpConnectionPtr> ConnectionList;
  typedef ThreadLocalSingleton<ConnectionList> LocalConnections;
  mutex mutex_;
  std::set<EventLoop*> loops_;
};

int main(int argc, char* argv[])
{
    EventLoop loop;
    uint16_t port = 2023;
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    server.setThreadNum(5);
    server.start();
    loop.loop();
}


