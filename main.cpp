#include "RtspServer.hpp"

int main() {
    RtspServer* server_handler = new RtspServer();
    server_handler->start_server();
    return 0;
}