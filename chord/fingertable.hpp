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

    /**
     * return the last node which is less than the given node
    */
    const Node &find(const Node &node)
    {
        /**
         * loop until the ith node is not between self and the given node
         *  because self < ith-node < node means ith-node is less than node
        */
        size_t i = 0;
        for (; i < M && nodes_[i].between(self_, node); ++i);

        /**
         * find will be called after checking the node is between self and its successor or not
         *  so i cannot be 0
        */
        assert(i != 0);

        return nodes_[i - 1];
    }

    void insert(const Node &node);

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
