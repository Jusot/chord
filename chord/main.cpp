#include "server.hpp"
#include "instruction.hpp"

#include <cassert>
#include <iostream>
#include <icarus/eventloopthread.hpp>

using namespace std;
using namespace icarus;
using namespace chord;

int main(int argc, char *argv[])
{
    assert(argc == 3);

    auto listen_ip = argv[1];
    auto listen_port = static_cast<std::uint16_t>(std::stoi(argv[2]));
    InetAddress listen_addr(listen_ip, listen_port);

    EventLoopThread loop_thread;
    Server server(loop_thread.start_loop(), listen_addr);
    server.start();

    while (true)
    {
        string instruction_str;
        std::getline(cin, instruction_str);

        auto res = Instruction::parse(instruction_str);
        if (!res.has_value())
        {
            cout << "<ERROR-INPUT>" << endl;
        }
        else
        {
            auto instruction = res.value();
            if (instruction.type() == Instruction::Quit)
            {
                /**
                 * quit: broadcast to set the predecessor's successor
                 *  and the successor's predecessor
                 *
                 * this will be handled in the destructor of server
                */
                return 0;
            }
            else
            {
                server.handle_instruction(instruction);
            }
        }
    }

    return 0;
}
