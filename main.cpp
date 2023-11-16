#include "RtspServer.hpp"
#include <boost/thread.hpp>

int main() {
    RtspServer* server_handler = new RtspServer();
    boost::thread* rtsp_server_thread_handler = new boost::thread(&RtspServer::start_server, server_handler);
    rtsp_server_thread_handler->join(); 
    delete server_handler;
    return 0;
}