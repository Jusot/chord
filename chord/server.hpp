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
                auto send_str = Message(Message::Join, { std::to_string(this->listen_addr_.to_port()) }).to_str();
                conn->send(send_str);
            }
        });
        client.set_message_callback([this, &finish] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
        {
            auto res = Message::parse(buf);
            if (!res.has_value())
            {
                return;
            }

            /**
             * as expected,
             *  it will return the successor of current node
            */
            auto msg = Message::parse(buf).value();
            Node successor(icarus::InetAddress(
                msg[0].c_str(),
                static_cast<std::uint16_t>(std::stoi(msg[1]))
            ));
            this->successor_ = successor;

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
        auto src_port = static_cast<std::uint16_t>(std::stoi(msg[0]));
        auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
        conn->send(find_successor(src_addr).to_str());
    }

    void on_message_notify(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        auto src_ip = conn->peer_address().to_ip();
        auto src_port = static_cast<std::uint16_t>(std::stoi(msg[0]));
        auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
        auto src_node = Node(src_addr);

        if (src_node.between(predecessor_, table_.self()))
        {
            predecessor_ = src_node;
        }

        auto send_str = Message(
            Message::Notify,
            {
                predecessor_.addr().to_ip(),
                std::to_string(predecessor_.addr().to_port())
            }
        ).to_str();
        conn->send(send_str);
    }

    void on_message_findsuc(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        auto src_ip = msg[0];
        auto src_port = static_cast<std::uint16_t>(std::stoi(msg[1]));
        icarus::InetAddress src_addr(src_ip.c_str(), src_port);
        conn->send(find_successor(src_addr).to_str());
    }

    void on_message_get(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    void on_message_put(const icarus::TcpConnectionPtr &conn, const Message &msg)
    {
        // ...
    }

    Message find_successor(const Node &node)
    {
        /**
         * if node is the direct successor
        */
        if (node.between(table_.self(), successor_))
        {
            return Message(
                Message::FindSuc,
                {
                    successor_.addr().to_ip(),
                    std::to_string(successor_.addr().to_port())
                }
            );
        }
        else
        {
            auto ask_node = table_.find(node);

            std::optional<Message> result;
            icarus::EventLoop loop;
            icarus::TcpClient client(&loop, ask_node.addr(), "chord client");

            client.set_connection_callback([this, node] (const icarus::TcpConnectionPtr &conn)
            {
                if (conn->connected())
                {
                    auto send_str = Message(
                        Message::FindSuc,
                        {
                            node.addr().to_ip(),
                            std::to_string(node.addr().to_port())
                        }
                    ).to_str();
                    conn->send(send_str);
                }
            });
            client.set_message_callback([this, &result, &loop] (const icarus::TcpConnectionPtr &conn, Buffer *buf)
            {
                auto res = Message::parse(buf);
                if (!res.has_value())
                {
                    return;
                }

                result = res;
                conn->shutdown();
                loop.quit();
            });

            client.connect();
            loop.loop();

            if (result.has_value())
            {
                return result.value();
            }
            else
            {
                /**
                 * TODO: fix finger table and refind
                */
                return result.value(); // ERROR NOW
            }
        }
    }

    void stabilize()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));

            /**
             * notify the successor, update the successor
             *  and fix the finger table
            */
            Node successor;
            icarus::EventLoop loop;
            icarus::TcpClient client(&loop, successor_.addr(), "chord client");

            client.set_connection_callback([this] (const icarus::TcpConnectionPtr &conn)
            {
                if (conn->connected())
                {
                    auto send_str = Message(
                        Message::Notify,
                        {
                            std::to_string(this->listen_addr_.to_port())
                        }
                    ).to_str();
                }
            });
            client.set_message_callback([this, &successor, &loop] (const icarus::TcpConnectionPtr &conn, Buffer *buf)
            {
                auto res = Message::parse(buf);
                if (!res.has_value())
                {
                    return;
                }

                auto msg = res.value();
                successor = Node(icarus::InetAddress(
                    msg[0].c_str(),
                    static_cast<std::uint16_t>(std::stoi(msg[1]))
                ));
                loop.quit();
            });

            client.connect();
            loop.loop();

            successor_ = successor;
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
