#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "message.hpp"
#include "instruction.hpp"
#include "fingertable.hpp"

#include <icarus/buffer.hpp>
#include <icarus/eventloop.hpp>
#include <icarus/tcpclient.hpp>
#include <icarus/tcpserver.hpp>
#include <icarus/tcpconnection.hpp>

namespace chord
{
class Server
{
  public:
    Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr)
      : listen_addr_(listen_addr)
      , tcp_server_(loop, listen_addr, "chord server")
    {
        // ...
    }

    /**
     * quit: broadcast to set the predecessor's successor
     *  and the successor's predecessor
    */
    ~Server()
    {
        // ...
    }

    void start()
    {
        tcp_server_.start();
    }

    void handle_instruction(const Instruction &ins)
    {
        switch (ins.type())
        {
        case Instruction::Join:
          handle_instruction_join(ins.value());
          break;

        case Instruction::Get:
          handle_instruction_get(ins.value());
          break;

        case Instruction::Put:
          handle_instruction_put(ins.value());
          break;

        default:
          break;
        }
    }

  private:
    void handle_instruction_join(const std::string &value)
    {
        auto pos = value.find(':');
        auto dst_ip = value.substr(0, pos);
        auto dst_port = static_cast<std::uint16_t>(std::stoi(value.substr(pos + 1)));
        auto src_port = listen_addr_.to_port();

        /**
         * wait here until things are ok or wrong
        */
        icarus::EventLoop loop;
        icarus::InetAddress server_addr(dst_ip.c_str(), dst_port);
        icarus::TcpClient client(&loop, server_addr, "chord client");

        /**
         * set callbacks
        */
        client.set_connection_callback([] (const icarus::TcpConnectionPtr &conn) {

        });
        client.set_message_callback([] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf) {

        });

        /**
         * loop until the process is finished
        */
        client.connect();
        loop.loop();
    }

    void handle_instruction_get(const std::string &value)
    {

    }

    void handle_instruction_put(const std::string &value)
    {

    }

    /**
     * join: insert it directly if received node is between the current node and the successor
    */
    void on_message(const icarus::TcpConnectionPtr &conn, Buffer *buf)
    {

    }

  private:
    Node predecessor_;
    Node successor_;
    FingerTable table_;
    InetAddress listen_addr_;
    icarus::TcpServer tcp_server_;
};
} // namespace chord

#endif
