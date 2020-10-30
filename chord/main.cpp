#include "server.hpp"

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
        string instruction;
        std::getline(cin, instruction);

        /**
         * Quit: broadcast to set the predecessor's successor
         *  and the successor's predecessor
        */
        if (instruction == "quit")
        {
            return 0;
        }
    }

    return 0;
}
