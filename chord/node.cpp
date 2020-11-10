#include "node.hpp"

namespace chord
{
Node::Node(const icarus::InetAddress &addr)
  : hash_(addr), addr_(addr)
{
    // ...
}

Node::Node(const Node &other)
  : hash_(other.hash_), addr_(other.addr_)
{
    // ...
}

bool Node::between(const HashType &pre, const HashType &suc) const
{
    return hash_.between(pre, suc);
}

bool Node::between(const Node &pre, const Node &suc) const
{
    return between(pre.hash_, suc.hash_);
}

bool Node::operator==(const Node &rhs) const
{
    return hash_ == rhs.hash_;
}

bool Node::operator!=(const Node &rhs) const
{
    return hash_ != rhs.hash_;
}

const HashType &Node::hash() const
{
    return hash_;
}

const icarus::InetAddress &Node::addr() const
{
    return addr_;
}
} // namespace chord
