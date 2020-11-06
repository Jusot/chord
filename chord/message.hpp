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
    /**
     * these annotations only for sending
    */
    enum Type : char
    {
        Join, // ,src_port >> ,suc_ip,peer_port
        Notify, // ,src_port >> ,pre_ip,pre_port
        FindSuc, // ,src_ip,src_port >> ,suc_ip,suc_port

        Get,
        Put,
    };

    static std::optional<Message> parse(const std::string &message)
    {
        Type type = Type(message[0]);
        if (type > Type::Put)
        {
            return {};
        }

        std::vector<std::string> params;
        std::size_t pos = 1;
        while (pos != message.npos)
        {
            auto next_pos = message.find(',', pos + 1);
            params.push_back(message.substr(pos + 1, next_pos - pos - 1));
            pos = next_pos;
        }

        return Message(type, std::move(params));
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
        // ...
    }

    std::string to_str() const
    {
        std::string result(1, char(type_));
        for (auto &param : params_)
        {
            result += ',' + param;
        }
        return result + "\r\n";
    }

    Type type() const { return type_; }
    const std::vector<std::string> &params() const { return params_; }
    const std::string &operator[](std::size_t ind_of_param) const
    { return params_[ind_of_param]; }

  private:
    Type type_;
    std::vector<std::string> params_;
};
} // namespace chord

#endif
