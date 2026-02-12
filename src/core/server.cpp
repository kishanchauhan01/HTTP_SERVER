#include "server.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <thread>

#include "socket.hpp"

using namespace std;

int acceptClient(Socket& listner) {
    // Create clientAddr
    sockaddr_in clientAddr;

    while (true) {
        int clientFd = listner.accept(clientAddr);

        if (clientFd >= 0) {
            return clientFd;  // success!
        }

        // --- ERROR Handling ---
        if (errno == EINTR)
            continue;  // Interrupted by signal(timer or a child process
                       // ending), try again immediately

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Non-blocking socket with no connection waiting
            this_thread::sleep_for(chrono::milliseconds(10));
            continue;
        }

        if (errno == EMFILE || errno == ENFILE) {
            // Server is too busy (too many open files)
            cerr << "Server busy (EMFILE). Pausing..." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }

        // If we are here then it is a Fatal err (EBADF, EINVAL)
        cerr << "Fatal accept error: " << strerror(errno) << endl;
        return -1;
    }
}

void handleClient() { cout << "client accept" << endl; }

Server::Server(int p) : port(p) {
    // Initialize obj of type Socket
    status = STARTED;
    listner = std::make_unique<Socket>();
    cout << "Server is: " << status << endl;
}

Server::~Server() { stop(); }

void Server::run() {
    status = RUNNING;
    cout << "Server is: " << status << endl;
    // Create listening socket
    bool isSockCreated = listner->create();

    if (!isSockCreated) {
        cerr << "run(): Failed to create socket by Socket::create() " << errno
             << strerror(errno) << endl;

        status = STOPPED;
        return;
    }

    // Bind the socket to port
    bool isBind = listner->bind(port);

    if (!isBind) {
        cerr << "run(): Failed to bind the socket by Socket::bind()" << errno
             << strerror(errno);

        status = STOPPED;
        return;
    }

    // Listen on the port
    bool isListen = listner->listen(20);

    if (!isListen) {
        cerr << "run(): Failed to listen to port by Socket::listen()" << errno
             << strerror(errno);

        status = STOPPED;
        return;
    }

    // Accept the client request
    while (status == RUNNING) {
        // Accept the client

        status = SLIPPING;
        int clientFd = acceptClient(*listner);

        status = RUNNING;
        if (clientFd == -1) {
            // Accept falied fatally
            cerr << "Server stopping due to fatal accept error." << endl;
            status = STOPPED;
            break;
        }

        cout << "New Client Connected! FD" << endl;

        thread(handleClient, clientFd);

        ::close(clientFd);
    }
}

void Server::stop() {
    status = STOPPED;
    cout << "Server is: " << status << endl;
}