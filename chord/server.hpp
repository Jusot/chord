#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "message.hpp"
#include "instruction.hpp"
#include "fingertable.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <icarus/buffer.hpp>
#include <icarus/eventloop.hpp>
#include <icarus/tcpclient.hpp>
#include <icarus/tcpserver.hpp>
#include <icarus/inetaddress.hpp>
#include <icarus/tcpconnection.hpp>

namespace chord
{
class Server
{
  public:
    Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr)
      : table_(std::hash<std::string>{}(listen_addr.to_ip_port()))
      , established_(false)
      , listen_addr_(listen_addr)
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
        bool finish = false;
        icarus::EventLoop loop;
        icarus::InetAddress server_addr(dst_ip.c_str(), dst_port);
        icarus::TcpClient client(&loop, server_addr, "chord client");

        /**
         * set callbacks
        */
        client.set_connection_callback([this] (const icarus::TcpConnectionPtr &conn)
        {
            /**
             * when connection is connected,
             *  we will send a message `join,src_port`
            */
            if (conn->connected())
            {
                auto send_str = Message(Message::Join, { std::to_string(this->listen_addr_.to_port()) }).to_str() + "\r\n";
                conn->send(send_str);
            }
        });
        client.set_message_callback([&finish] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
        {
            auto res = Message::parse(buf);
            if (!res.has_value())
            {
                return;
            }

            /**
             * TODO: resolve message
            */
            conn->shutdown();
            finish = true;
        });
        client.enable_retry();

        std::cout << "CONNECTING..." << std::endl;
        std::thread timer([&loop]
        {
            /**
             * TODO: unsafe, may involve more states
            */
            std::this_thread::sleep_for(std::chrono::seconds(3));
            loop.quit();
        });
        timer.detach();

        /**
         * loop until the process is finished
        */
        client.connect();
        loop.loop();

        /**
         * after loop quits
        */
        if (finish)
        {
            std::cout << "[ESTABILISHED SUCCESSFULLY]" << std::endl;

            established_ = true;
        }
        else
        {
            std::cout << "[FAILED CONNECTION]" << std::endl;
        }
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
        auto res = Message::parse(buf);
        if (!res.has_value())
        {
            return;
        }

        auto &message = res.value();
        switch (message.type())
        {
        case Message::Join:
            on_message_join(conn, message);
            break;
        case Message::Insert:
            on_message_insert(conn, message);
            break;
        case Message::Get:
            on_message_get(conn, message);
            break;
        case Message::Put:
            on_message_put(conn, message);
            break;
        }
    }

    void on_message_join(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        auto src_ip = conn->peer_address().to_ip();
        auto src_port = static_cast<std::uint16_t>(std::stoi(msg[0]));
        auto peer_addr = icarus::InetAddress(src_ip.c_str(), src_port);

        Node node(peer_addr);
        if (table_.find(node))
        {
            return;
        }

        if (predecessor_.has_value())
        {
            /**
             * this is detached
             *  and we assume the predecessor is alive
            */
            send_message(predecessor_.addr(), Message(
                Message::Insert,
                {
                    peer_addr.to_ip(),
                    std::to_string(peer_addr.to_port())
                })
            );

            if (predecessor_.hash_value() < node.hash_value() && node.hash_value() < table_.base())
            {
                predecessor_ = node;
            }
        }
        else
        {
            predecessor_ = node;
        }

        if (successor_.has_value())
        {

        }
        else
        {
            successor_ = node;
        }

        auto send_str = Message(Message::Join, {"success"}).to_str() + "\r\n";
        conn->send(send_str);
    }

    void on_message_insert(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {

    }

    void on_message_delete(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {

    }

    void on_message_get(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    void on_message_put(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    /**
     * only send message in a detached thread
     *  and the peer is assumed alive so that we just send
    */
    void send_message(const icarus::InetAddress &addr, const Message &msg)
    {
        std::thread send_thread([=]
        {
            icarus::EventLoop loop;
            icarus::TcpClient client(&loop, addr, "chord client");

            /**
             * set callbacks
            */
            client.set_connection_callback([=] (const icarus::TcpConnectionPtr &conn)
            {
                conn->send(msg.to_str() + "\r\n");
            });
            client.set_write_complete_callback([&] (const icarus::TcpConnectionPtr &conn)
            {
                conn->shutdown();
                loop.quit();
            });

            /*
            std::thread timer([&loop]
            {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                loop.quit();
            });
            timer.detach();
            */

            client.connect();
            loop.loop();
        });

        send_thread.detach();
    }

  private:
    Node predecessor_;
    Node successor_;
    FingerTable table_;

    bool established_;
    InetAddress listen_addr_;
    icarus::TcpServer tcp_server_;
};
} // namespace chord

#endif
