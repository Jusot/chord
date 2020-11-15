#include "client.hpp"
#include "server.hpp"
#include "instruction.hpp"

#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <iostream>
#include <icarus/buffer.hpp>
#include <icarus/tcpclient.hpp>

namespace chord
{
Server::Server(icarus::EventLoop *loop, const icarus::InetAddress &listen_addr)
  : predecessor_(listen_addr)
  , table_(listen_addr)
  , established_(false)
  , loop_(loop)
  , listen_addr_(listen_addr)
  , tcp_server_(loop, listen_addr, "chord server")
{
    tcp_server_.set_thread_num(10);
    tcp_server_.set_message_callback([this] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
    {
        this->on_message(conn, buf);
    });
}

void Server::start()
{
    tcp_server_.start();
}

/**
 * quit: broadcast to set the predecessor's successor
 *  and the successor's predecessor
*/
void Server::stop()
{
    if (!established_)
    {
        return;
    }
    established_ = false;

    if (successor() != self())
    {
        Client(successor().addr()).send(Message(
            Message::PreQuit, predecessor_.addr()
        ));
    }

    if (predecessor_ != self())
    {
        Client(predecessor_.addr()).send(Message(
            Message::SucQuit, successor().addr()
        ));
    }
}

void Server::handle_instruction(const Instruction &ins)
{
    std::lock_guard lock(mutex_);

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

    case Instruction::Quit:
        handle_instruction_quit();
        break;

    case Instruction::SelfBoot:
        handle_instruction_selfboot();
        break;

    case Instruction::Print:
        handle_instruction_print();
        break;
    }
}

void Server::handle_instruction_join(const std::string &value)
{
    auto pos = value.find(':');
    auto dst_ip = value.substr(0, pos);
    auto dst_port = static_cast<std::uint16_t>(std::stoi(value.substr(pos + 1)));
    auto dst_addr = icarus::InetAddress(dst_ip.c_str(), dst_port);

    std::cout << "[CONNECTING]" << std::endl;

    Client client(dst_addr, std::chrono::seconds(1));
    auto result = client.send_and_wait_response(Message(
        Message::Join, listen_addr_.to_port()
    ));

    if (result.has_value())
    {
        auto msg = result.value();

        Node successor(msg.param_as_addr());
        this->successor() = successor;
        this->table_.insert(successor);

        std::cout << "[ESTABILISHED SUCCESSFULLY]" << std::endl;
        std::cout << "[SUCCESSOR] Is " << successor.addr().to_ip_port() << std::endl;
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

void Server::handle_instruction_get(const std::string &value)
{
    auto hash = std::hash<std::string>{}(value);
    auto server_addr = find_successor(hash).param_as_addr();
    if (HashType(server_addr) == HashType(listen_addr_))
    {
        return;
    }

    std::cout << "[GET] File whose hash is " << hash << std::endl;

    /**
     * filename cannot involve ','
     *  and assume the file exists
    */
    std::thread get_thread([server_addr, filename = value]
    {
        Client client(server_addr);
        std::ofstream out(filename);
        if (!client.send_and_wait_stream(Message(filename), out))
        {
            std::cout << "[FAILED GET] No such file: " << filename << std::endl;
        }
        else
        {
            std::cout << "[GET SUCCESSFULLY] Download file: " << filename << std::endl;
        }
    });
    get_thread.detach();
}

void Server::handle_instruction_put(const std::string &value)
{
    auto hash = std::hash<std::string>{}(value);
    auto peer_addr = find_successor(hash).param_as_addr();
    if (HashType(peer_addr) != HashType(listen_addr_))
    {
        std::cout << "[PUT] File whose hash is " << hash
            << " to node " << peer_addr.to_ip_port() << std::endl
        ;
        Client(peer_addr).send(Message(listen_addr_.to_port(), value));
    }
}

void Server::handle_instruction_quit()
{
    stop();
    loop_->quit();
}

void Server::handle_instruction_selfboot()
{
    established_ = true;
    std::thread stabilize_thread([this]
    {
        this->stabilize();
    });
    stabilize_thread.detach();

    std::cout << "[SELF BOOT]" << std::endl;
}

void Server::handle_instruction_print()
{
    std::cout << "[PRINT] Self is " << listen_addr_.to_ip_port()
        << "\n[PRINT] Predecessor is " << predecessor_.addr().to_ip_port()
        << "\n[PRINT] Successor is " << successor().addr().to_ip_port();

    auto &nodes = table_.nodes();
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        std::cout << "\n[PRINT] |" << i << "|" << nodes[i].hash().to_str() << "|" << nodes[i].addr().to_ip_port();
    }
    std::cout << std::endl;
}

void Server::on_message(const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
{
    if (!established_)
    {
        conn->force_close();
        return;
    }

    std::lock_guard lock(mutex_);

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
    case Message::FindSuc:
        on_message_findsuc(conn, message);
        break;

    case Message::PreNotify:
        on_message_prenotify(conn, message);
        break;
    case Message::SucNotify:
        on_message_sucnotify(conn, message);
        break;

    case Message::PreQuit:
        on_message_prequit(conn, message);
        break;
    case Message::SucQuit:
        on_message_sucquit(conn, message);
        break;

    case Message::Get:
        on_message_get(conn, message);
        break;
    case Message::Put:
        on_message_put(conn, message);
        break;
    }

    conn->shutdown();
}

void Server::on_message_join(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    auto src_ip = conn->peer_address().to_ip();
    auto src_port = msg.param_as_port();
    auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
    conn->send(find_successor(src_addr).to_str());

    std::cout << "[RECEIVE JOIN] From " << src_addr.to_ip_port() << std::endl;
}

void Server::on_message_prenotify(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    auto src_ip = conn->peer_address().to_ip();
    auto src_port = msg.param_as_port();
    auto src_addr = icarus::InetAddress(src_ip.c_str(), src_port);
    auto src_node = Node(src_addr);

    if (src_node.between(predecessor_, self()))
    {
        update_predecessor(src_node);
    }

    conn->send(Message(Message::PreNotify, predecessor_.addr()).to_str());

    // std::cout << "[RECEIVE NOTIFY] From " << src_addr.to_ip_port() << std::endl;
}

/**
 * only keep alive
*/
void Server::on_message_sucnotify(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    conn->send(Message(Message::SucNotify, successor().addr()).to_str());
}

void Server::on_message_findsuc(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    /**
     * msg[0] is the hash value
    */
    conn->send(find_successor(msg.param_as_hash()).to_str());

    std::cout << "[RECEIVE FindSuc] Finds " << msg[0] << std::endl;
}

void Server::on_message_prequit(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    table_.remove(predecessor_);
    update_predecessor(msg.param_as_addr());
}

void Server::on_message_sucquit(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    table_.remove(successor());
    update_successor(msg.param_as_addr());
}

void Server::on_message_get(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    std::cout << "[RECEIVE Get] Of file " << msg[0] << std::endl;

    /**
     * assume the file exists
    */
    std::ifstream file(msg[0]);

    if (!file.is_open())
    {
        return;
    }

    std::string data;
    while (!file.eof())
    {
        data.push_back(file.get());
    }
    /**
     * see https://en.cppreference.com/w/cpp/io/basic_ios/eof
    */
    data.pop_back();

    conn->send(data);
}

void Server::on_message_put(const icarus::TcpConnectionPtr &conn, const Message &msg)
{
    std::cout << "[RECEIVE Put] Of file " << msg[1] << std::endl;

    auto server_ip = conn->peer_address().to_ip();
    auto server_port = msg.param_as_port();
    auto server_addr = icarus::InetAddress(server_ip.c_str(), server_port);

    std::thread get_thread([server_addr, filename = msg[1]]
    {
        Client client(server_addr);
        std::ofstream out(filename);
        client.send_and_wait_stream(Message(filename), out);
    });
    get_thread.detach();
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
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (!established_)
        {
            continue;
        }

        {
            std::lock_guard lock(mutex_);
            notify_predecessor();
            notify_successor();
            fix_finger_table();
        }
    }
}

/**
 * check predecessor alive or not
*/
void Server::notify_predecessor()
{
    if (predecessor_ == self())
    {
        return;
    }

    Client client(predecessor_.addr(), std::chrono::seconds(1));
    auto result = client.send_and_wait_response(Message(
        Message::SucNotify,
        listen_addr_.to_port()
    ));

    if (!result.has_value())
    {
        table_.remove(predecessor_);
        update_predecessor(table_.find_closest_pre(self()));
    }
}

void Server::notify_successor()
{
    /**
     * check the predecessor directly if the successor is self
    */
    if (successor() == self())
    {
        auto new_successor = predecessor_;
        if (new_successor.between(self(), successor()))
        {
            update_successor(new_successor);
        }
        return;
    }

    // std::cout << "[CHECK SUCCESSOR] i.e. " << successor_.addr().to_ip_port() << std::endl;

    /**
     * notify the successor, update the successor
     *  and fix the finger table
    */
    Client client(successor().addr(), std::chrono::seconds(1));
    auto result = client.send_and_wait_response(Message(
        Message::PreNotify,
        listen_addr_.to_port()
    ));

    if (result.has_value())
    {
        auto msg = result.value();
        Node new_successor(msg.param_as_addr());

        if (new_successor.between(self(), successor()))
        {
            update_successor(new_successor);
        }
    }
    else
    {
        table_.remove(successor());
        update_successor(table_.find_closest_suc(self()));
    }
}

/**
 * FIXME: Unstable now
*/
void Server::fix_finger_table()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, FingerTable::M - 1);

    auto ind = dis(gen);
    Node node(find_successor(self().hash() + (1ull << ind)).param_as_addr());
    table_[ind] = node;
}

/**
 * FIXME: Unstable now
*/
Message Server::find_successor(const HashType &hash)
{
    /**
     * if self <= hash < successor
     *  return the direct successor
    */
    if (hash == self().hash() || hash.between(self().hash(), successor().hash()))
    {
        return Message(Message::FindSuc, successor().addr());
    }
    else
    {
        // Client client(successor().addr(), std::chrono::seconds(1));
        // auto result = client.send_and_wait_response(Message(
        //     Message::FindSuc, hash
        // ));
        // if (result.has_value())
        // {
        //     return result.value();
        // }
        // else
        // {
        //     return Message(Message::FindSuc, successor().addr());
        // }

        auto ask_node = table_.find_closest_pre(hash);
        /**
         * if the hash's successor is not the direct successor
         *  and cannot find another node which is closed to the hash
         *  then ask the direct successor
        */
        if (ask_node == self())
        {
            ask_node = successor();
        }

        Client client(ask_node.addr(), std::chrono::seconds(1));
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
             * if the node is dead then remove it and refind
            */
            table_.remove(ask_node);
            return find_successor(hash);
        }
    }
}

const Node &Server::self() const
{
    return table_.self();
}

Node &Server::successor()
{
    return table_[0];
}

void Server::update_predecessor(const Node &new_predecessor)
{
    if (predecessor_ == new_predecessor)
    {
        return;
    }

    std::cout << "[UPDATE PREDECESSOR] To " << new_predecessor.addr().to_ip_port() << std::endl;

    predecessor_ = new_predecessor;
    table_.insert(new_predecessor);
}

void Server::update_successor(const Node &new_successor)
{
    if (successor() == new_successor)
    {
        return;
    }

    std::cout << "[UPDATE SUCCESSOR] To " << new_successor.addr().to_ip_port() << std::endl;

    successor() = new_successor;
    table_.insert(new_successor);
}
} // namespace chord
