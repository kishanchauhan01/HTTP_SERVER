#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>

#include <memory>
#include <string>

#include "socket.hpp"

enum serverStatus { STARTED, RUNNING, SLIPPING, STOPPED };

class Server {
   public:
    Server(int port);

    Server(const Server&) = delete;
    Server operator=(const Server&) = delete;

    ~Server();

    void run();
    void stop();

   private:
    int port;
    serverStatus status = STOPPED;  // by default server is stop
    std::unique_ptr<Socket> listner;
};

#endif
