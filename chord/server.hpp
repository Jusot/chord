#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
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

    void start()
    {
        tcp_server_.start();
    }

    /**
     * Quit:
    */
    void on_connection(const icarus::TcpConnectionPtr &conn)
    {
        if (!conn->connected())
        {

        }
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
