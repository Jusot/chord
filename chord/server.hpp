#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "message.hpp"
#include "instruction.hpp"
#include "fingertable.hpp"

#include <icarus/tcpserver.hpp>
#include <icarus/tcpconnection.hpp>

namespace chord
{
class Server
{
  public:
    Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr)
      : tcp_server_(loop, listen_addr, "chord server")
    {
        // ...
    }

    /**
     * Quit: broadcast to set the predecessor's successor
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

    }

    void handle_instruction_get(const std::string &value)
    {

    }

    void handle_instruction_put(const std::string &value)
    {

    }

    /**
     * Join: insert it directly if received node is between the current node and the successor
    */
    void on_message(const icarus::TcpConnectionPtr &conn, Buffer *buf)
    {

    }

  private:
    Node predecessor_;
    Node successor_;
    FingerTable table_;
    icarus::TcpServer tcp_server_;
};
} // namespace chord

#endif
