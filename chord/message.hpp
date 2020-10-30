#ifndef __CHORD_MESSAGE_HPP__
#define __CHORD_MESSAGE_HPP__

#include <string>

namespace chord
{
class Message
{
  public:
    enum Type
    {
        Join,
        Quit,

        Get,
        Put,

        SetPredecessor,
        SetSuccessor,
    };

  private:
    std::string method_;
};
} // namespace chord

#endif
