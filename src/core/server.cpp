#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <memory>
#include <string>

#include "socket.hpp"

using namespace std;

Server::Server(int p) : port(p) {
    // Initialize obj of type Socket
    listner = std::make_unique<Socket>();
}

Server::~Server() { stop(); }

void Server::run() {
    status = RUNNING;
    // Create listening socket
    bool isSockCreated = listner->create();

    if (!isSockCreated) {
        cerr << "run(): Failed to create socket by Socket::create() " << errno
             << strerror(errno) << endl;

        return;
    }

    // Bind the socket to port
    bool isBind = listner->bind(port);

    if (!isBind) {
        cerr << "run(): Failed to bind the socket by Socket::bind()" << errno
             << strerror(errno);

        return;
    }

    // Listen on the port
    bool isListen = listner->listen(20);

    if (!isListen) {
        cerr << "run(): Failed to listen to port by Socket::listen()" << errno
             << strerror(errno);

        return;
    }

    // Accept the client request
    while (status == RUNNING) {
    }
}

void Server::stop() { status = STOPPED; }