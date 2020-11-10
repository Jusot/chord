#ifndef __CHORD_NODE_HPP__
#define __CHORD_NODE_HPP__

#include "message.hpp"
#include "hashtype.hpp"

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
    Node() = delete;

    explicit Node(const icarus::InetAddress &addr)
      : hash_(addr), addr_(addr)
    {
        // ...
    }

    Node(const Node &other)
      : hash_(other.hash_), addr_(other.addr_)
    {
        // ...
    }

    const HashType &hash() const
    {
        return hash_;
    }

    const icarus::InetAddress &addr() const
    {
        return addr_;
    }

    bool operator==(const Node &rhs) const
    {
        return hash_ == rhs.hash_;
    }

    bool operator!=(const Node &rhs) const
    {
        return hash_ != rhs.hash_;
    }

    bool between(const HashType &pre, const HashType &suc) const
    {
        return hash_.between(pre, suc);
    }

    bool between(const Node &pre, const Node &suc) const
    {
        return between(pre.hash_, suc.hash_);
    }

  private:
    HashType hash_;
    icarus::InetAddress addr_;
};
} // namespace chord

#endif
