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
    Node() : has_value_(false)
    {
        // ...
    }

    Node(const icarus::InetAddress &addr)
      : has_value_(true)
      , hash_value_(std::hash<std::string>{}(addr.to_ip_port()))
      , addr_(addr)
    {
        // ...
    }

    Node(const Node &other)
      : has_value_(other.has_value_)
      , hash_value_(other.hash_value_)
      , addr_(other.addr_)
    {
        // ...
    }

    bool has_value() const
    {
        return has_value_;
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

    bool between(const Node &pre, const Node &suc) const
    {
        return between(pre.hash_value_, suc.hash_value_);
    }

    /**
     * >.>.>.>.>.>.>.>.>.>.
     * pre .<. this .<. suc
     *
     * 0        ||        0
     * >.>.>.> this .>.>.>.
     * suc .<.<.<.<.<.< pre
    */
    bool between(std::size_t pre, std::size_t suc) const
    {
        if (pre == suc)
        {
            return true;
        }
        else if (pre < suc)
        {
            return pre < hash_value_ && hash_value_ < suc;
        }
        else
        {
            return pre < hash_value_ || hash_value_ < suc;
        }
    }

  private:
    bool has_value_;
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
