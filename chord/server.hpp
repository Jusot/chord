#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "message.hpp"
#include "fingertable.hpp"

#include <mutex>
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

    void start();
    void stop();

    void handle_instruction(const Instruction &ins);

  private:
    void handle_instruction_join(const std::string &value);
    void handle_instruction_get (const std::string &value);
    void handle_instruction_put (const std::string &value);
    void handle_instruction_quit();
    void handle_instruction_selfboot();
    void handle_instruction_print();

    void on_message(const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf);
    void on_message_join      (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_findsuc   (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_prenotify (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_sucnotify (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_prequit   (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_sucquit   (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_get       (const icarus::TcpConnectionPtr &conn, const Message &msg);
    void on_message_put       (const icarus::TcpConnectionPtr &conn, const Message &msg);

    void stabilize();
    void notify_predecessor();
    void notify_successor();
    void fix_finger_table();

    Message find_successor(const HashType &hash);

    const Node &self() const;
    Node &successor();
    void update_predecessor(const Node &new_predecessor);
    void update_successor(const Node &new_successor);

  private:
    Node predecessor_;
    FingerTable table_;

    bool established_;
    icarus::EventLoop *loop_;
    icarus::InetAddress listen_addr_;
    icarus::TcpServer tcp_server_;

    std::mutex mutex_;
};
} // namespace chord

#endif
