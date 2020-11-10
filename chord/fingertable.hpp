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
    const Node &find(const Node &node) const;
    const Node &find(const HashType &hash) const;
    void insert(const Node &node);
    void remove(const Node &node);

    const Node &self() const;
    const std::vector<Node> &nodes() const;

  private:
    Node self_;
    std::vector<Node> nodes_;
};
} // namespace chord

#endif
