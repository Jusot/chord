#ifndef __CHORD_CLIENT_HPP__
#define __CHORD_CLIENT_HPP__

#include "message.hpp"

#include <chrono>
#include <thread>
#include <icarus/buffer.hpp>
#include <icarus/callbacks.hpp>
#include <icarus/tcpclient.hpp>
#include <icarus/eventloop.hpp>
#include <icarus/inetaddress.hpp>

namespace chord
{
using TimeoutCallback = std::function<void(bool timeout, const std::optional<Message> &result)>;

/**
 * wrapper of TcpClient
 *  provide the timeout scheme
*/
class Client
{
  public:
    Client(icarus::InetAddress server_addr)
      : keep_wait_(true)
      , server_addr_(server_addr)
    {
        // ...
    }

    Client(icarus::InetAddress server_addr,
        std::chrono::seconds time)
      : keep_wait_(false)
      , timeout_(time)
      , server_addr_(server_addr)
    {
        // ...
    }

    /**
     * just send msg in a detached thread
     *  and don't care it is successful or not
    */
    void send(const Message &msg)
    {
        std::thread send_thread([server_addr = server_addr_, msg]
        {
            icarus::EventLoop loop;
            icarus::TcpClient client(&loop, server_addr, "chord client");

            client.enable_retry();
            client.set_connection_callback([&msg] (const icarus::TcpConnectionPtr &conn)
            {
                if (conn->connected())
                {
                    conn->send(msg.to_str());
                }
            });

            client.set_write_complete_callback([&loop] (const icarus::TcpConnectionPtr &conn)
            {
                conn->shutdown();
                loop.quit();
            });

            client.connect();
            /**
             * loop may not stop
             *  but this will be blocked by os
            */
            loop.loop();
        });
        send_thread.detach();
    }

    /**
     * if timeout is set
     *  res will be return and passed to the timeout callback
    */
    std::optional<Message>
    send_and_wait_response(const Message &msg)
    {
        std::optional<Message> result;
        icarus::EventLoop loop;
        icarus::TcpClient client(&loop, server_addr_, "chord client");

        client.enable_retry();
        client.set_connection_callback([&msg] (const icarus::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                conn->send(msg.to_str());
            }
        });

        if (keep_wait_)
        {
            client.set_message_callback([&result, &loop] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
            {
                if (buf->findCRLF() == nullptr)
                {
                    return;
                }

                result = Message::parse(buf);
                conn->shutdown();
                loop.quit();
            });

            client.connect();
            loop.loop();
        }
        else
        {
            bool timeout = true;
            client.set_message_callback([&result, &loop, &timeout] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
            {
                if (buf->findCRLF() == nullptr)
                {
                    return;
                }

                result = Message::parse(buf);
                conn->shutdown();
                loop.quit();
                timeout = false;
            });

            std::thread timer([&loop, time = timeout_]
            {
                /**
                 * TODO: unsafe, may involve more states
                */
                std::this_thread::sleep_for(time);
                loop.quit();
            });
            timer.detach();

            client.connect();
            loop.loop();

            timeout_callback_(timeout, result);
        }

        return result;
    }

    void set_timeout(std::chrono::seconds time)
    {
        keep_wait_ = false;
        timeout_ = time;
    }

    void set_timeout_callback(TimeoutCallback cb)
    {
        timeout_callback_ = std::move(cb);
    }

    void keep_wait()
    {
        keep_wait_ = true;
    }

  private:
    bool keep_wait_;
    std::chrono::seconds timeout_;
    icarus::InetAddress server_addr_;
    TimeoutCallback timeout_callback_;
};
} // namespace chord

#endif
