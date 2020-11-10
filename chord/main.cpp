#include "server.hpp"
#include "instruction.hpp"

#include <cassert>
#include <iostream>
#include <icarus/inetaddress.hpp>
#include <icarus/eventloopthread.hpp>

using namespace chord;

int main(int argc, char *argv[])
{
    assert(argc == 3);

    auto listen_ip = argv[1];
    auto listen_port = static_cast<std::uint16_t>(std::stoi(argv[2]));
    icarus::InetAddress listen_addr(listen_ip, listen_port);

    icarus::EventLoop loop;
    Server server(&loop, listen_addr);

    std::thread input_thread([&loop, &server]
    {
        while (true)
        {
            std::string instruction_str;
            std::getline(std::cin, instruction_str);
            if (instruction_str.empty())
            {
                continue;
            }

            auto res = Instruction::parse(instruction_str);
            if (!res.has_value())
            {
                std::cout << "<ERROR> Unknown Input" << std::endl;
            }
            else
            {
                auto instruction = res.value();
                if (instruction.type() == Instruction::Quit)
                {
                    break;
                }
                server.handle_instruction(instruction);
            }
        }
    });

    std::cout << "[SERVER STARTED] At " << listen_addr.to_ip_port() << std::endl;

    server.start();
    loop.loop();

    return 0;
}
