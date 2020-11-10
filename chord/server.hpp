#ifndef __CHORD_SERVER_HPP__
#define __CHORD_SERVER_HPP__

#include "node.hpp"
#include "client.hpp"
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
      : predecessor_(listen_addr)
      , successor_(listen_addr)
      , table_(listen_addr)
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
        if (!established_)
        {
            return;
        }

        if (successor_ != table_.self())
        {
            Client(successor_.addr()).send(Message(
                Message::PreQuit, predecessor_.addr()
            ));
        }

        if (predecessor_ != table_.self())
        {
            Client(predecessor_.addr()).send(Message(
                Message::SucQuit, successor_.addr()
            ));
        }
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
        auto dst_addr = icarus::InetAddress(dst_ip.c_str(), dst_port);

        Client client(dst_addr, std::chrono::seconds(3));

        client.set_timeout_callback([this] (bool timeout, const std::optional<Message> &result)
        {
            if (!timeout)
            {
                /**
                 * assume the result is correct
                */
                auto msg = result.value();

                Node successor(msg.param_as_addr());
                this->successor_ = successor;

                std::cout << "[ESTABILISHED SUCCESSFULLY]" << std::endl;
                established_ = true;

                /**
                 * TODO: init finger table from the successor
                */

                std::thread stabilize_thread([this]
                {
                    this->stabilize();
                });
                stabilize_thread.detach();
            }
            else
            {
                std::cout << "[FAILED CONNECTION]" << std::endl;
            }
        });

        std::cout << "CONNECTING..." << std::endl;
        client.send_and_wait_response(Message(
            Message::Join, listen_addr_.to_port()
        ));
    }

    void handle_instruction_get(const std::string &value)
    {

    }

    void handle_instruction_put(const std::string &value)
    {

    }

    void on_message(const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
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
        case Message::Notify:
            on_message_notify(conn, message);
            break;
        case Message::FindSuc:
            on_message_findsuc(conn, message);
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
        auto src_port = msg.param_as_port();
        auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
        conn->send(find_successor(msg.param_as_addr()).to_str());
    }

    void on_message_notify(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        auto src_ip = conn->peer_address().to_ip();
        auto src_port = msg.param_as_port();
        auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
        auto src_node = Node(src_addr);

        if (src_node.between(predecessor_, table_.self()))
        {
            predecessor_ = src_node;
        }

        conn->send(Message(Message::Notify, predecessor_.addr()).to_str());
    }

    void on_message_findsuc(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        /**
         * msg[0] is the hash value
        */
        conn->send(find_successor(msg.param_as_hash()).to_str());
    }

    void on_message_get(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    void on_message_put(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    Message find_successor(const HashType &hash)
    {
        /**
         * if hash is between self and the direct successor
        */
        if (hash.between(table_.self().hash(), successor_.hash()))
        {
            return Message(Message::FindSuc, successor_.addr());
        }
        else
        {
            auto ask_node = table_.find(hash);
            Client client(ask_node.addr());

            auto result = client.send_and_wait_response(Message(
                Message::FindSuc, hash
            ));

            if (result.has_value())
            {
                return result.value();
            }
            else
            {
                /**
                 * what happened
                */
                abort();
            }
        }
    }

    /**
     * in stabilization:
     *  1. ask the predecessor of the successor
     *  2. update the successor by the got predecessor
     *      if the predecessor of the successor is not self
    */
    void stabilize()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));

            /**
             * check the predecessor directly if the successor is self
            */
            if (successor_ == table_.self())
            {
                auto successor = predecessor_;
                if (successor.between(table_.self(), successor_))
                {
                    successor_ = successor;
                }
            }
            else
            {
                /**
                 * notify the successor, update the successor
                 *  and fix the finger table
                */
                Client client(successor_.addr());
                auto result = client.send_and_wait_response(Message(
                    Message::Notify,
                    listen_addr_.to_port()
                ));

                if (result.has_value())
                {
                    auto msg = result.value();
                    auto successor = Node(msg.param_as_addr());

                    if (successor.between(table_.self(), successor_))
                    {
                        successor_ = successor;
                    }
                }
                /**
                 * else
                */
            }

            fix_finger_table();
        }
    }

    void fix_finger_table()
    {

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
