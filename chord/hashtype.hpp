#ifndef __CHORD_HASHTYPE_HPP__
#define __CHORD_HASHTYPE_HPP__

#include <string>
#include <cstddef>
#include <icarus/inetaddress.hpp>

namespace chord
{
/**
 * simple wrapper of std::size_t
*/
class HashType
{
  public:
    HashType(std::size_t value);
    HashType(const icarus::InetAddress &addr);
    HashType(const HashType &other);
    // `operator=` is implicitly-declared

    bool between(HashType pre, HashType suc) const;

    bool operator==(const HashType &rhs) const;
    bool operator!=(const HashType &rhs) const;
    bool operator<(const HashType &rhs) const;
    bool operator<=(const HashType &rhs) const;
    HashType operator+(const HashType &rhs) const;
    HashType operator-(const HashType &rhs) const;

    std::size_t value() const;
    std::string to_str() const;

  private:
    /**
     * hash_value is calculated by the standard function hash simply
     *  instead of using sha-1 or sha-256
     *  and assume that its size is 64-bits
    */
    std::size_t value_;
};
} // namespace chord

#endif
