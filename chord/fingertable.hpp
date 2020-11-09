#ifndef __CHORD_FINGERTABLE_HPP__
#define __CHORD_FINGERTABLE_HPP__

#include "node.hpp"
#include "hashtype.hpp"

#include <vector>
#include <cassert>

namespace chord
{
class FingerTable
{
  public:
    // assume that each byte is 8-bit
    static constexpr std::size_t M = sizeof(std::size_t) * 8;

  public:
    FingerTable(const icarus::InetAddress &addr)
      : self_(addr)
      , nodes_(M, Node(addr))
    {
        // ...
    }

    const Node &find(const Node &node) const
    {
        return find(node.hash());
    }

    /**
     * return the last node which is less than the given hash
    */
    const Node &find(const HashType &hash) const
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

    /**
     * simply traverse each position in current version
    */
    void insert(const Node &node)
    {
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

    const Node &self() const
    {
        return self_;
    }

    const std::vector<Node> &nodes() const
    {
        return nodes_;
    }

  private:
    Node self_;
    std::vector<Node> nodes_;
};
} // namespace chord

#endif
