#include "message.hpp"

namespace chord
{
std::optional<Message> Message::parse(const std::string &message)
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

std::optional<Message> Message::parse(icarus::Buffer *buf)
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

Message::Message(Type type, std::uint16_t port)
  : type_(type)
  , params_({std::to_string(port)})
{
    // ...
}

Message::Message(Type type, const icarus::InetAddress &addr)
  : type_(type)
  , params_({addr.to_port(), std::to_string(addr.to_port())})
{
    // ...
}

Message::Message(Type type, const HashType &hash)
  : type_(type)
  , params_({hash.to_str()})
{
    // ...
}

std::string Message::to_str() const
{
    std::string result(1, char(type_));
    for (auto &param : params_)
    {
        result += ',' + param;
    }
    return result + "\r\n";
}

std::uint16_t Message::param_as_port(std::size_t i) const
{
    return static_cast<uint16_t>(std::stoi(params_[i]));
}

icarus::InetAddress Message::param_as_addr(std::size_t start) const
{
    return icarus::InetAddress(
        params_[start].c_str(),
        param_as_port(start + 1)
    );
}

HashType Message::param_as_hash(std::size_t i) const
{
    return HashType(std::stoull(params_[i]));
}

Message::Type Message::type() const
{
    return type_;
}

const std::vector<std::string> &Message::params() const
{
    return params_;
}

const std::string &Message::operator[](std::size_t ind_of_param) const
{
    return params_[ind_of_param];
}

Message::Message(Type type, std::vector<std::string> params)
  : type_(type)
  , params_(std::move(params))
{
    // ...
}
} // namespace chord
