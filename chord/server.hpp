#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "message.hpp"
#include "fingertable.hpp"

#include <icarus/eventloop.hpp>
#include <icarus/tcpserver.hpp>
#include <icarus/inetaddress.hpp>
#include <icarus/tcpconnection.hpp>

namespace chord
{
class Instruction;
class Server
{
  public:
    Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr);
    ~Server();

    void start();
    void handle_instruction(const Instruction &ins);

  private:
    void handle_instruction_join(const std::string &value);
    void handle_instruction_get (const std::string &value);
    void handle_instruction_put (const std::string &value);

    void on_message(const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf);
    void on_message_join    (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_notify  (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_findsuc (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_prequit (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_sucquit (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_get     (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_put     (const icarus::TcpConnectionPtr &conn, const Message &msg);

    void stabilize();
    void fix_finger_table();

    Message find_successor(const HashType &hash);

  private:
    Node predecessor_;
    Node successor_;
    FingerTable table_;

    bool established_;
    icarus::InetAddress listen_addr_;
    icarus::TcpServer tcp_server_;
};
} // namespace chord

#endif
