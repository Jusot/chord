#ifndef __CHORD_HASHTYPE_HPP__
#define __CHORD_HASHTYPE_HPP__

#include <string>
#include <cctype>
#include <functional>
#include <icarus/inetaddress.hpp>

namespace chord
{
/**
 * simple wrapper of std::size_t
*/
class HashType
{
  public:
    HashType(std::size_t value)
      : value_(value)
    {
        // ...
    }

    HashType(const std::string &value)
      : HashType(std::stoull(value))
    {
        // ...
    }

    HashType(const icarus::InetAddress &addr)
      : HashType(std::hash<std::string>{}(addr.to_ip_port()))
    {
        // ...
    }

    HashType(const HashType &other)
      : value_(other.value_)
    {
        // ...
    }

    /**
     * >.>.>.>.>.>.>.>.>.>.
     * pre .<. this .<. suc
     *
     * 0        ||        0
     * >.>.>.> this .>.>.>.
     * suc .<.<.<.<.<.< pre
    */
    bool between(HashType pre, HashType suc) const
    {
        if (pre == suc)
        {
            return true;
        }
        else if (pre < suc)
        {
            return pre.value_ < value_ && value_ < suc.value_;
        }
        else
        {
            return pre.value_ < value_ || value_ < suc.value_;
        }
    }

    bool operator==(const HashType &rhs) const
    {
        return value_ == rhs.value_;
    }

    bool operator<(const HashType &rhs) const
    {
        return value_ < rhs.value_;
    }

    HashType operator+(const HashType &rhs) const
    {
        return HashType(value_ + rhs.value_);
    }

    std::size_t value() const
    {
        return value_;
    }

    std::string to_str() const
    {
        return std::to_string(value_);
    }

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
