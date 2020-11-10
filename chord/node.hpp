#ifndef __CHORD_NODE_HPP__
#define __CHORD_NODE_HPP__

#include "hashtype.hpp"

#include <icarus/inetaddress.hpp>

namespace chord
{
class Node
{
  public:
    Node() = delete;
    Node(const icarus::InetAddress &addr);
    Node(const Node &other);
    Node &operator=(const Node &other);

    bool between(const HashType &pre, const HashType &suc) const;
    bool between(const Node &pre, const Node &suc) const;

    bool operator==(const Node &rhs) const;
    bool operator!=(const Node &rhs) const;

    const HashType &hash() const;
    const icarus::InetAddress &addr() const;

  private:
    HashType hash_;
    icarus::InetAddress addr_;
};
} // namespace chord

#endif
