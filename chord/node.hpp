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
