#ifndef __CHORD_MESSAGE_HPP__
#define __CHORD_MESSAGE_HPP__

#include <string>
#include <vector>
#include <optional>
#include <icarus/buffer.hpp>

namespace chord
{
/**
 * for messages, they are like `type,p1,p2,...,pn`
*/
class Message
{
  public:
    enum Type
    {
        Join, // join,src_port

        Get,
        Put,

        SetPre,
        SetSuc,
    };

    static std::optional<Message> parse(const std::string &message)
    {
        std::size_t pos = message.find(',');

        Type type;
        auto method = message.substr(0, pos);
        std::vector<std::string> params;

        if (method == "join")
        {
            type = Join;
        }
        else if (method == "get")
        {
            type = Get;
        }
        else if (method == "put")
        {
            type = Put;
        }
        else if (method == "setpre")
        {
            type = SetPre;
        }
        else if (method == "setsuc")
        {
            type = SetSuc;
        }
        else
        {
            return {};
        }

        while (pos != message.npos)
        {
            auto next_pos = message.find(',', pos + 1);
            params.push_back(message.substr(pos + 1, next_pos - pos - 1));
            pos = next_pos;
        }

        return Message(type, std::move(method), std::move(params));
    }

    static std::optional<Message> parse(Buffer *buf)
    {
        auto crlf = buf->findCRLF();
        if (crlf == nullptr)
        {
            return {};
        }

        auto message = parse(buf->retrieve_as_string(crlf - buf->peek()));
        buf->retrieve_until(crlf + 2);

        return message;
    }

    Message(Type type, std::vector<std::string> params)
      : type_(type), params_(std::move(params))
    {
        switch (type)
        {
        case Join:
            method_ = "join";
            break;
        case Get:
            method_ = "get";
            break;
        case Put:
            method_ = "put";
            break;
        case SetPre:
            method_ = "setpre";
            break;
        case SetSuc:
            method_ = "setsuc";
            break;
        }
    }

    Message(Type type, std::string method, std::vector<std::string> params)
      : type_(type)
      , method_(std::move(method))
      , params_(std::move(params))
    {
        // ...
    }

    std::string to_str() const
    {
        auto result = method_;
        for (auto &param : params_)
        {
            result += ',' + param;
        }
        return result;
    }

    Type type() const { return type_; }
    const std::string &method() const { return method_; }
    const std::vector<std::string> &params() const { return params_; }
    const std::string &operator[](std::size_t ind_of_param) const
    { return params_[ind_of_param]; }

  private:
    Type type_;
    std::string method_;
    std::vector<std::string> params_;
};
} // namespace chord

#endif
