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

        client.set_connection_callback([&msg, &loop] (const icarus::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                conn->send(msg.to_str());
                conn->shutdown();
            }
            else
            {
                loop.quit();
            }
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

    std::thread send_thread([this, &msg, &result]
    {
        enum State
        {
            Connected,
            DisConnected,
        };

        State state = DisConnected;

        icarus::EventLoop loop;
        icarus::TcpClient client(&loop, server_addr_, "chord client");

        client.set_connection_callback([&msg, &state, &loop] (const icarus::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                state = Connected;
                conn->send(msg.to_str());
                conn->shutdown();
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
                loop.quit();
            });

            client.connect();
            loop.loop();
        }
        else
        {
            client.set_message_callback([&result, &loop] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
            {
                if (buf->findCRLF() == nullptr)
                {
                    return;
                }

                result = Message::parse(buf);
                loop.quit();
            });

            std::thread timer([&state, &loop, &client, time = timeout_]
            {
                auto start = std::chrono::high_resolution_clock::now();
                while (std::chrono::high_resolution_clock::now() - start < time)
                {
                    if (state == Connected)
                    {
                        break;
                    }
                }

                if (state == DisConnected)
                {
                    client.stop();
                    /**
                     * wait client stoped
                    */
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    loop.quit();
                }
            });

            client.connect();
            loop.loop();
            timer.join();
        }
    });
    send_thread.join();

    return result;
}

bool Client::send_and_wait_stream(const Message &msg, std::ostream &out)
{
    bool receive_stream = false;

    std::thread send_thread([this, &msg, &out, &receive_stream]
    {
        icarus::EventLoop loop;
        icarus::TcpClient client(&loop, server_addr_, "chord client");

        client.set_connection_callback([&msg, &loop] (const icarus::TcpConnectionPtr &conn)
        {
            if (conn->connected())
            {
                conn->send(msg.to_str());
                conn->shutdown();
            }
            else
            {
                loop.quit();
            }
        });

        client.set_message_callback([&out, &receive_stream, &loop] (const icarus::TcpConnectionPtr &conn, icarus::Buffer *buf)
        {
            out.write(buf->peek(), buf->readable_bytes());
            buf->retrieve_all();

            receive_stream = true;
        });

        client.connect();
        loop.loop();
    });
    send_thread.join();

    return receive_stream;
}

void Client::set_timeout(std::chrono::seconds time)
{
    keep_wait_ = false;
    timeout_ = time;
}

void Client::keep_wait()
{
    keep_wait_ = true;
}
} // namespace chord
