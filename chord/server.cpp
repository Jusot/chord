#include "client.hpp"
#include "server.hpp"
#include "instruction.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <icarus/buffer.hpp>
#include <icarus/tcpclient.hpp>

namespace chord
{
Server::Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr)
  : predecessor_(listen_addr)
  , successor_(listen_addr)
  , table_(listen_addr)
  , established_(false)
  , listen_addr_(listen_addr)
  , tcp_server_(loop, listen_addr, "chord server")
{
    tcp_server_.set_thread_num(10);
    tcp_server_.set_message_callback([this] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
    {
        this->on_message(conn, buf);
    });
}

/**
 * quit: broadcast to set the predecessor's successor
 *  and the successor's predecessor
*/
Server::~Server()
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

void Server::start()
{
    tcp_server_.start();
}

void Server::handle_instruction(const Instruction &ins)
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

void Server::handle_instruction_join(const std::string &value)
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

void Server::handle_instruction_get(const std::string &value)
{

}

void Server::handle_instruction_put(const std::string &value)
{

}

void Server::on_message(const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
{
    if (!established_)
    {
        return;
    }

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

    case Message::PreQuit:
        on_message_prequit(conn,message);
        break;
    case Message::SucQuit:
        on_message_sucquit(conn,message);
        break;

    case Message::Get:
        on_message_get(conn, message);
        break;
    case Message::Put:
        on_message_put(conn, message);
        break;
    }
}

void Server::on_message_join(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    auto src_ip = conn->peer_address().to_ip();
    auto src_port = msg.param_as_port();
    auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
    conn->send(find_successor(msg.param_as_addr()).to_str());
}

void Server::on_message_notify(const icarus::TcpConnectionPtr &conn, const Message &msg)
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

void Server::on_message_findsuc(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    /**
     * msg[0] is the hash value
    */
    conn->send(find_successor(msg.param_as_hash()).to_str());
}

void Server::on_message_prequit(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    predecessor_ = msg.param_as_addr();
}

void Server::on_message_sucquit(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    table_.remove(successor_);
    successor_ = msg.param_as_addr();
    table_.insert(successor_);
}

void Server::on_message_get(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    // ...
}

void Server::on_message_put(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    // ...
}

/**
 * in stabilization:
 *  1. ask the predecessor of the successor
 *  2. update the successor by the got predecessor
 *      if the predecessor of the successor is not self
*/
void Server::stabilize()
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
                Node successor(msg.param_as_addr());

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

void Server::fix_finger_table()
{

}

Message Server::find_successor(const HashType &hash)
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
} // namespace chord
