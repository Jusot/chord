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

const Node &FingerTable::find_closest_pre(const Node &node) const
{
    return find_closest_pre(node.hash());
}

const Node &FingerTable::find_closest_pre(const HashType &hash) const
{
    constexpr std::size_t max = -1;

    std::size_t ind = 0;
    HashType dis = max;
    for (std::size_t i = 0; i < M; ++i)
    {
        auto now = nodes_[i].hash();
        if (now <= hash)
        {
            if (hash - now < dis)
            {
                dis = hash - now;
                ind = i;
            }
        }
        else if (hash < now)
        {
            if (hash + max - now < dis)
            {
                dis = hash + max - now;
                ind = i;
            }
        }
    }

    if (dis == max)
    {
        return self_;
    }
    return nodes_[ind];
}

const Node &FingerTable::find_closest_suc(const Node &node) const
{
    return find_closest_suc(node.hash());
}

const Node &FingerTable::find_closest_suc(const HashType &hash) const
{
    constexpr std::size_t max = -1;

    std::size_t ind = 0;
    HashType dis = max;
    for (std::size_t i = 0; i < M; ++i)
    {
        auto now = nodes_[i].hash();
        /**
         * different from find_closest_pre
         *  don't consider the same case
        */
        if (hash < now)
        {
            if (now - hash < dis)
            {
                dis = now - hash;
                ind = i;
            }
        }
        else if (now < hash)
        {
            if (now + max - hash < dis)
            {
                dis = now + max - hash;
                ind = i;
            }
        }
    }

    if (dis == max)
    {
        return self_;
    }
    return nodes_[ind];
}

void FingerTable::insert(Node node)
{
    /**
     * simply traverse each position in current version
    */
    auto base = self_.hash();
    for (std::size_t i = 0; i < M; ++i)
    {
        auto start = base + (1ull << i);
        // statr <= node < nodes[i]
        if (node.hash() == start || node.between(start, nodes_[i].hash()))
        {
            nodes_[i] = node;
        }
    }
}

void FingerTable::remove(Node node)
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
