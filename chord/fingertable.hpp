#ifndef __CHORD_FINGERTABLE_HPP__
#define __CHORD_FINGERTABLE_HPP__

#include "node.hpp"

#include <vector>

namespace chord
{
class HashType;
class FingerTable
{
  public:
    // assume that each byte is 8-bit
    static constexpr std::size_t M = sizeof(std::size_t) * 8;

  public:
    FingerTable(const icarus::InetAddress &addr);

    /**
     * return the last node which is less than the given hash
    */
    const Node &find_closest_pre(const Node &node) const;
    const Node &find_closest_pre(const HashType &hash) const;
    const Node &find_closest_suc(const Node &node) const;
    const Node &find_closest_suc(const HashType &hash) const;
    /**
     * cannot use const reference to Node
     *  because it may be the node in this table
    */
    void insert(Node node);
    void remove(Node node);

    const Node &self() const;
    const std::vector<Node> &nodes() const;
    Node &operator[](std::size_t ind);

  private:
    Node self_;
    std::vector<Node> nodes_;
};
} // namespace chord

#endif
