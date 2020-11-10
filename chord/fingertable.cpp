#include "hashtype.hpp"
#include "fingertable.hpp"

#include <cassert>

namespace chord
{
FingerTable::FingerTable(const icarus::InetAddress &addr)
  : self_(addr)
  , nodes_(M, Node(addr))
{
    // ...
}

const Node &FingerTable::find(const Node &node) const
{
    return find(node.hash());
}

const Node &FingerTable::find(const HashType &hash) const
{
    std::size_t ind = 0, dis = -1;
    constexpr std::size_t max = -1;
    for (std::size_t i = 0; i < M; ++i)
    {
        auto now = nodes_[i].hash().value();
        if (now <= hash.value())
        {
            if (hash.value() - now < dis)
            {
                dis = hash.value() - now;
                ind = i;
            }
        }
        else
        {
            if (hash.value() + max - now < dis)
            {
                dis = hash.value() + max - now;
                ind = 1;
            }
        }
    }
    return nodes_[ind];
}

void FingerTable::insert(const Node &node)
{
    /**
     * simply traverse each position in current version
    */
    auto base = self_.hash();
    for (std::size_t i = 0; i < M; ++i)
    {
        auto anchor = base + (1ull << i);
        if (node.between(anchor, nodes_[i].hash()))
        {
            nodes_[i] = node;
        }
    }
}

void FingerTable::remove(const Node &node)
{
    /**
     * simply substitute node by self
    */
    for (std::size_t i = 0; i < M; ++i)
    {
        if (nodes_[i] == node)
        {
            nodes_[i] = self_;
        }
    }
}

const Node &FingerTable::self() const
{
    return self_;
}

const std::vector<Node> &FingerTable::nodes() const
{
    return nodes_;
}

Node &FingerTable::operator[](std::size_t ind)
{
    return nodes_[ind];
}
} // namespace chord
