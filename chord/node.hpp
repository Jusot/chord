#ifndef __CHORD_NODE_HPP__
#define __CHORD_NODE_HPP__

#include <string>
#include <functional>
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

    bool has_value() const
    {
        return has_value_;
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
