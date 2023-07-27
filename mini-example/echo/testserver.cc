#include <string>

#include "TcpServer.h"

#include "AsyncLogging.h"
#include <iostream>
using namespace std;

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : server_(loop, addr, name), loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));

        server_.setWriteCompleteCallback(
            std::bind(&EchoServer::WriteCompleteCallback,this));

        server_.setThreadInitCallback(
            std::bind(&EchoServer::initthread,this));    

        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的subloop线程数量
        server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG(LoggerLevel::INFO,"Connection UP : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG(LoggerLevel::INFO,"Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
        }
    }
    
    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        LOG(LoggerLevel::INFO,"testserver:onMessage",NULL);
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }

    void WriteCompleteCallback(){
        LOG(LoggerLevel::INFO,"write finish!",NULL);
    }

    void initthread(){
        LOG(LoggerLevel::INFO,"init thread finish!",NULL);
    }

    EventLoop *loop_;
    TcpServer server_;
};

void printt(string a){
    cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<a<<endl;
}
int maiin()
{
    
    EventLoop loop;
    InetAddress addr(8002);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    LOG_INIT(".", LoggerLevel::INFO);
    loop.runAfter(1, std::bind(printt, "once1"));
    loop.runAfter(1.5, std::bind(printt, "once1.5"));
    loop.runAfter(5.5, std::bind(printt, "once5.5"));
    loop.runEvery(3,std::bind(printt,"every3"));
    loop.runAfter(3.5, std::bind(printt, "once3.5"));
    loop.runAfter(4, std::bind(printt,"once4"));
    Timer* ptimer = loop.runAfter(10, std::bind(printt, "once10"));
    loop.cancelTimer(ptimer);
    loop.loop();
    return 0;
}