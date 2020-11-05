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
    FingerTable(std::size_t base)
      : base_(base)
    {
        // ...
    }

  private:
    std::size_t base_;
    std::array<Node, M> nodes_;
};
} // namespace chord

#endif
