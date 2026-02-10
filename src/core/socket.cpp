#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <socket.hpp>

using namespace std;

// Constructor
Socket::Socket() : socketFd(-1) {}

// Destructor
Socket::~Socket() { close(); }

// Create socket
bool Socket::create() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd < 0) {
        cerr << "create(): Failed to create socket (" << errno << ")"
             << strerror(errno) << endl;
        return false;
    }

    // socketOption
    int opt = 1;

    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "create(): Failed to set SO_REUSEADDR (" << errno << ")"
             << strerror(errno) << endl;

        // Close the socket and reset
        ::close(socketFd);
        socketFd = -1;

        return false;
    }

    return true;
}

// Bind socket
bool Socket::bind(int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(socketFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "bind(): Bind Failed (" << errno << ")" << strerror(errno)
             << endl;

        // Close the socket and reset
        ::close(socketFd);
        socketFd = -1;

        return false;
    }

    return true;
}

// Listen on socket
bool Socket::listen(int backlog) {
    if (::listen(socketFd, backlog) < 0) {
        cerr << "listen(): Failed to listen on socket (" << errno << ")"
             << strerror(errno) << endl;

        return false;
    }

    return true;
}

// Accetp connection
int Socket::accept(sockaddr_in& clientAddr) {
    socklen_t len = sizeof(clientAddr);

    int newConnFd = ::accept(socketFd, (struct sockaddr*)&clientAddr, &len);

    if (newConnFd < 0) {
        cerr << "accept(): Failed to accept (" << errno << ")"
             << strerror(errno) << endl;
        return -1;
    }

    return newConnFd;
}

// Close socket
void Socket::close() {
    if (socketFd != -1) {
        ::close(socketFd);
        socketFd = -1;
    }
}

int Socket::getFd() const { return socketFd; }