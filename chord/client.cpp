#include "client.hpp"

#include <thread>
#include <icarus/buffer.hpp>
#include <icarus/callbacks.hpp>
#include <icarus/tcpclient.hpp>
#include <icarus/eventloop.hpp>

namespace chord
{
Client::Client(icarus::InetAddress server_addr)
  : keep_wait_(true)
  , server_addr_(server_addr)
{
    // ...
}

Client::Client(icarus::InetAddress server_addr,
    std::chrono::seconds time)
  : keep_wait_(false)
  , timeout_(time)
  , server_addr_(server_addr)
{
    // ...
}

void Client::send(const Message &msg)
{
    std::thread send_thread([server_addr = server_addr_, msg]
    {
        icarus::EventLoop loop;
        icarus::TcpClient client(&loop, server_addr, "chord client");

        client.enable_retry();
        client.set_connection_callback([&msg] (const icarus::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                conn->send(msg.to_str());
            }
        });

        client.set_write_complete_callback([&loop] (const icarus::TcpConnectionPtr &conn)
        {
            conn->shutdown();
            loop.quit();
        });

        client.connect();
        /**
         * loop may not stop
         *  but this will be blocked by os
        */
        loop.loop();
    });
    send_thread.detach();
}

std::optional<Message>
Client::send_and_wait_response(const Message &msg)
{
    std::optional<Message> result;
    icarus::EventLoop loop;
    icarus::TcpClient client(&loop, server_addr_, "chord client");

    client.enable_retry();
    client.set_connection_callback([&msg, &loop] (const icarus::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            conn->send(msg.to_str());
        }
        else
        {
            loop.quit();
        }
    });

    if (keep_wait_)
    {
        client.set_message_callback([&result, &loop] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
        {
            if (buf->findCRLF() == nullptr)
            {
                return;
            }

            result = Message::parse(buf);
            conn->shutdown();
            loop.quit();
        });

        client.connect();
        loop.loop();
    }
    else
    {
        bool timeout = true;
        client.set_message_callback([&result, &loop, &timeout] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
        {
            if (buf->findCRLF() == nullptr)
            {
                return;
            }

            result = Message::parse(buf);
            conn->shutdown();
            loop.quit();
            timeout = false;
        });

        std::thread timer([&loop, time = timeout_]
        {
            /**
             * TODO: unsafe, may involve more states
            */
            std::this_thread::sleep_for(time);
            loop.quit();
        });
        timer.detach();

        client.connect();
        loop.loop();

        timeout_callback_(timeout, result);
    }

    return result;
}

void Client::send_and_wait_stream(const Message &msg, std::ostream &out)
{
    icarus::EventLoop loop;
    icarus::TcpClient client(&loop, server_addr_, "chord client");

    client.enable_retry();
    client.set_connection_callback([&msg, &loop] (const icarus::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            conn->send(msg.to_str());
        }
        else
        {
            loop.quit();
        }
    });

    client.set_message_callback([&out, &loop] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
    {
        auto data = buf->retrieve_all_as_string();
        out.write(data.c_str(), data.size());
    });

    client.connect();
    loop.loop();
}

void Client::set_timeout(std::chrono::seconds time)
{
    keep_wait_ = false;
    timeout_ = time;
}

void Client::set_timeout_callback(TimeoutCallback cb)
{
    timeout_callback_ = std::move(cb);
}

void Client::keep_wait()
{
    keep_wait_ = true;
}
} // namespace chord
