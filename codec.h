#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include <functional>
#include <stdint.h>
#include <endian.h>
#include <string>

#include "AsyncLogging.h"
#include "Buffer.h"
#include "noncopyable.h"
#include "TcpConnection.h"
#include "Timestamp.h"
#include "Socket.h"


class LengthHeaderCodec : noncopyable
{
 public:
  typedef std::function<void (const TcpConnectionPtr&,
                                const string& message,
                                Timestamp)> StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
    : messageCallback_(cb)
  {
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime)
  {
    while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
    {
      // FIXME: use Buffer::peekInt32()
      const void* data = buf->peek();
      int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
      const int32_t len = be32toh(be32);
      if (len > 65536 || len < 0)
      {
        LOG(LoggerLevel::ERROR, "Invalid length %d",len);
        conn->shutdown();  // FIXME: disable reading
        break;
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        string message(buf->peek(), len);
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(len);
      }
      else
      {
        break;
      }
    }
  }

  // FIXME: TcpConnectionPtr
  void send(TcpConnection* conn,
            const string& message,
            int length)
  {
    string buf="";
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = htobe32(len);
    buf+=to_string(static_cast<int>(be32));
    buf+=message;
    conn->send(buf);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
