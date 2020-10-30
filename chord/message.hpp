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

        Get,
        Put,

        /**
         * these two only used between nodes
        */
        SetPredecessor,
        SetSuccessor,

        Wrong,
    };

    Message(const std::string &message)
    {
        std::size_t pos = message.find(' ');
        method_ = message.substr(0, pos);
        value_ = message.substr(pos + 1);

        if (method_ == "join")
        {
            type_ = Join;
        }
        else if (method_ == "get")
        {
            type_ = Get;
        }
        else if (method_ == "put")
        {
            type_ = Put;
        }
        else if (method_ == "set-p")
        {
            type_ = SetPredecessor;
        }
        else if (method_ == "set-s")
        {
            type_ = SetSuccessor;
        }
        else
        {
            type_ = Wrong;
        }
    }

  private:
    Type type_;
    std::string method_;
    std::string value_;
};
} // namespace chord

#endif
