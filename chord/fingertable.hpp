#ifndef __CHORD_FINGERTABLE_HPP__
#define __CHORD_FINGERTABLE_HPP__

#include "node.hpp"

#include <array>

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
    const Node &find(const Node &node);

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
