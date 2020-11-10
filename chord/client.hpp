#ifndef __CHORD_CLIENT_HPP__
#define __CHORD_CLIENT_HPP__

#include "message.hpp"

#include <chrono>
#include <optional>
#include <functional>
#include <icarus/inetaddress.hpp>

namespace chord
{
using TimeoutCallback = std::function<void(bool timeout, const std::optional<Message> &result)>;
/**
 * wrapper of TcpClient
 *  provide the timeout scheme
*/
class Client
{
  public:
    Client(icarus::InetAddress server_addr);
    Client(icarus::InetAddress server_addr, std::chrono::seconds time);

    /**
     * just send msg in a detached thread
     *  and don't care it is successful or not
    */
    void send(const Message &msg);
    /**
     * if timeout is set
     *  res will be return and passed to the timeout callback
    */
    std::optional<Message>
    send_and_wait_response(const Message &msg);

    void set_timeout(std::chrono::seconds time);
    void set_timeout_callback(TimeoutCallback cb);
    void keep_wait();

  private:
    bool keep_wait_;
    std::chrono::seconds timeout_;
    icarus::InetAddress server_addr_;
    TimeoutCallback timeout_callback_;
};
} // namespace chord

#endif
