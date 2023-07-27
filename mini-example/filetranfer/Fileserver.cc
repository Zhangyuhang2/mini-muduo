#include <string>

#include "TcpServer.h"
#include "define.h"


#include "AsyncLogging.h"

#include <iostream>

#include <stdio.h>
#include <unistd.h>


void onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
   LOG(LoggerLevel::INFO,"HighWaterMark   num: %d",17);
}

const int kBufSize = 64*1024;
const char* g_file = NULL;
typedef std::shared_ptr<FILE> FilePtr;

void onConnection(const TcpConnectionPtr& conn)
{
    
  if (conn->connected())
  {
    conn->setHighWaterMarkCallback(onHighWaterMark, kBufSize+1);

    FILE* fp = ::fopen(g_file, "rb");
    if (fp)
    {
      FilePtr ctx(fp, ::fclose);
      conn->setContext(ctx);
      char buf[kBufSize];
      size_t nread = ::fread(buf, 1, sizeof buf, fp);
      conn->send(buf);
    }
    else
    {
      cout<<"open file fail"<<endl;
      conn->shutdown();
      LOG(LoggerLevel::INFO,"FileServer - no such file  num: %d",43);
    }
  }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
  
   const FilePtr& fp = boost::any_cast<const FilePtr&>(conn->getContext());
  char buf[kBufSize];
  size_t nread = ::fread(buf, 1, sizeof buf, get_pointer(fp));
  if (nread > 0)
  {
    conn->send(buf);
  }
  else
  {
    conn->shutdown();
    LOG(LoggerLevel::INFO,"FileServer - done  num: %d",60);
  }
}

int maaaain(int argc, char* argv[])
{
  
    g_file = "zhangyuhang.txt";

    EventLoop loop;
    InetAddress listenAddr(2021);
    TcpServer server(&loop, listenAddr, "FileServer");
    server.setConnectionCallback(onConnection);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();
    loop.loop();

}

