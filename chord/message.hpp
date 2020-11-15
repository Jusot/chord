#ifndef __CHORD_MESSAGE_HPP__
#define __CHORD_MESSAGE_HPP__

#include "hashtype.hpp"

#include <string>
#include <vector>
#include <optional>
#include <icarus/buffer.hpp>
#include <icarus/inetaddress.hpp>

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
        Join, // ,src_port >> ,suc_ip,suc_port
        FindSuc, // ,hash_value >> ,suc_ip,suc_port

        PreNotify, // ,src_port >> ,pre_ip,pre_port
        SucNotify, // ,src_port >> ,suc_ip,suc_ip

        PreQuit, // ,pre_ip,pre_port
        SucQuit, // ,suc_ip,suc_port

        Get, // ,file_name >> data
        Put, // ,src_port,file_name
    };

    static std::optional<Message> parse(const std::string &message);
    static std::optional<Message> parse(icarus::Buffer *buf);

  public:
    explicit Message(Type type, std::uint16_t port);
    explicit Message(Type type, const icarus::InetAddress &addr);
    explicit Message(Type type, const HashType &hash);
    explicit Message(const std::string &filename);
    explicit Message(std::uint16_t port, const std::string &filename);

    std::string to_str() const;
    std::uint16_t       param_as_port(std::size_t i = 0) const;
    icarus::InetAddress param_as_addr(std::size_t start = 0) const;
    HashType            param_as_hash(std::size_t i = 0) const;

    Type type() const;
    const std::vector<std::string> &params() const;
    const std::string &operator[](std::size_t ind_of_param) const;

  private:
    Message(Type type, std::vector<std::string> params);

    Type type_;
    std::vector<std::string> params_;
};
} // namespace chord

#endif
