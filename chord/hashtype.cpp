#include "hashtype.hpp"

#include <functional>

namespace chord
{
HashType::HashType(std::size_t value)
  : value_(value)
{
    // ...
}

HashType::HashType(const icarus::InetAddress &addr)
  : HashType(std::hash<std::string>{}(addr.to_ip_port()))
{
    // ...
}

HashType::HashType(const HashType &other)
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
 *
 * in (pre, suc]
*/
bool HashType::between(HashType pre, HashType suc) const
{
    if (pre == suc)
    {
        return true;
    }
    else if (pre < suc)
    {
        return pre.value_ < value_ && value_ <= suc.value_;
    }
    else
    {
        return pre.value_ < value_ || value_ <= suc.value_;
    }
}

bool HashType::operator==(const HashType &rhs) const
{
    return value_ == rhs.value_;
}

bool HashType::operator!=(const HashType &rhs) const
{
    return value_ != rhs.value_;
}

bool HashType::operator<(const HashType &rhs) const
{
    return value_ < rhs.value_;
}

HashType HashType::operator+(const HashType &rhs) const
{
    return HashType(value_ + rhs.value_);
}

std::size_t HashType::value() const
{
    return value_;
}

std::string HashType::to_str() const
{
    return std::to_string(value_);
}
} // namespace chord
