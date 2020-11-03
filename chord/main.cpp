#include "server.hpp"
#include "instruction.hpp"

#include <iostream>
#include <icarus/eventloopthread.hpp>

using namespace std;
using namespace icarus;
using namespace chord;

int main()
{
    EventLoopThread loop_thread;
    InetAddress listen_addr(6666);
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
                 * Quit: broadcast to set the predecessor's successor
                 *  and the successor's predecessor
                 *
                 * This will be handled in the destructor of server
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
