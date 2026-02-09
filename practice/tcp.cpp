#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);

    if (listening == -1) {
        cerr << "Can't create a socket!";
        return -1;
    }

    // Bind the socket to a IP / PORT
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);  // host to network short
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (struct sockaddr*)&hint, sizeof(struct sockaddr)) ==
        -1) {
        cerr << "Can't bind to IP/port" << endl;
        return -2;
    }

    // Mark the socket for listening in
    if (listen(listening, SOMAXCONN) == -1) {
        cerr << "Can't listen" << endl;
        return -3;
    }

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket =
        accept(listening, (struct sockaddr*)&client, &clientSize);

    if (clientSocket == -1) {
        cerr << "Problem with client connecting!" << endl;
        return -4;
    }

    // close the listening socket
    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((struct sockaddr*)&client, sizeof(client), host,
                             NI_MAXHOST, svc, NI_MAXSERV, 0);

    if (result) {
        cout << host << "connected on" << svc << endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << "connected on" << ntohs(client.sin_port) << endl;
    }

    // while receving - display message, echo message
    char buf[4096];

    while (true) {
        // Clear the buffer
        memset(buf, 0, 4096);
        // wait for a message
        int bytesRecv = recv(clientSocket, buf, 4096, 0);

        if (bytesRecv == -1) {
            cerr << "There was a connection issue" << endl;
            break;
        }

        if (bytesRecv == 0) {
            cerr << "The client disconnected" << endl;
            break;
        }

        // display message
        cout << "Received: " << string(buf, 0, bytesRecv) << endl;

        // resend message
        send(clientSocket, buf, bytesRecv+1, 0);
    }

    // close the socket
    close(clientSocket);
 
    return 0;
}