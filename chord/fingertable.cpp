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
    /**
     * loop until the ith node is not between self and the given hash_value
     *  because self < ith-node < hash_value means ith-node is less than hash_value
    */
    size_t i = 0;
    for (; i < M && nodes_[i].between(self_.hash(), hash); ++i);

    /**
     * find will be called after checking the hash_value is between self and its successor or not
     *  so i cannot be 0
    */
    assert(i != 0);
    return nodes_[i - 1];
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
        if (node.hash() == anchor)
        {
            nodes_[i] = node;
        }
        else if (node.between(anchor, nodes_[i].hash()))
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
} // namespace chord
