#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

class Socket {
   public:
    Socket();

    // Disable copy constructor
    Socket(const Socket&) = delete;

    // Disable copy assingment
    Socket operator=(const Socket&) = delete;

    ~Socket();

    bool create();
    bool bind(int port);
    bool listen(int backlog);

    int accept(sockaddr_in& clientAddr);

    void close();

    int getFd() const;

   private:
    int socketFd;
};

#endif