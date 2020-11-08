#ifndef __CHORD_FINGERTABLE_HPP__
#define __CHORD_FINGERTABLE_HPP__

#include "node.hpp"

#include <array>
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
    {
        for (auto &node : nodes_)
        {
            node = Node(addr);
        }
    }

    const Node &find(const Node &node) const
    {
        return find(node.hash_value());
    }

    /**
     * return the last node which is less than the given hash_value
    */
    const Node &find(std::size_t hash_value) const
    {
        /**
         * loop until the ith node is not between self and the given hash_value
         *  because self < ith-node < hash_value means ith-node is less than hash_value
        */
        size_t i = 0;
        for (; i < M && nodes_[i].between(self_.hash_value(), hash_value); ++i);

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
        auto base = self_.hash_value();
        for (std::size_t i = 0; i < M; ++i)
        {
            auto anchor = base + (1ull << i);
            if (node.hash_value() == anchor)
            {
                nodes_[i] = node;
            }
            else if (node.between(anchor, nodes_[i].hash_value()))
            {
                nodes_[i] = node;
            }
        }
    }

    const Node &self() const
    {
        return self_;
    }

    const std::array<Node, M> &nodes() const
    {
        return nodes_;
    }

  private:
    Node self_;
    std::array<Node, M> nodes_;
};
} // namespace chord

#endif
