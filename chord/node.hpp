#ifndef __CHORD_NODE_HPP__
#define __CHORD_NODE_HPP__

#include "message.hpp"

#include <string>
#include <thread>
#include <functional>
#include <icarus/eventloop.hpp>
#include <icarus/tcpclient.hpp>
#include <icarus/inetaddress.hpp>

namespace chord
{
class Node
{
  public:
    Node() = default;

    Node(const icarus::InetAddress &addr)
      : hash_value_(std::hash<std::string>{}(addr.to_ip_port()))
      , addr_(addr)
    {
        // ...
    }

    Node(const Node &other)
      : hash_value_(other.hash_value_)
      , addr_(other.addr_)
    {
        // ...
    }

    /**
     * only send in a detached thread
    */
    void send_message(const Message &msg)
    {
        std::thread send_thread([=]
        {
            icarus::EventLoop loop;
            icarus::TcpClient client(&loop, addr_, "chord client");

            /**
             * set callbacks
            */
            client.set_connection_callback([=] (const icarus::TcpConnectionPtr &conn)
            {
                conn->send(msg.to_str() + "\r\n");
            });
            client.set_write_complete_callback([&] (const icarus::TcpConnectionPtr &conn)
            {
                conn->shutdown();
                loop.quit();
            });

            /*
            std::thread timer([&loop]
            {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                loop.quit();
            });
            timer.detach();
            */

            client.connect();
            loop.loop();
        });

        send_thread.detach();
    }

    std::size_t hash_value() const
    {
        return hash_value_;
    }

    const icarus::InetAddress &addr() const
    {
        return addr_;
    }

    bool operator==(const Node &rhs) const
    {
        return hash_value_ == rhs.hash_value_;
    }

    bool between(const Node &lnode, const Node &rnode) const
    {

    }

  private:
    /**
     * hash_value is calculated by the standard function hash simply
     *  instead of using sha-1 or sha-256
     *  and assume that its size is 64-bits
    */
    std::size_t hash_value_;
    icarus::InetAddress addr_;
};
} // namespace chord

#endif
