#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // Required for getaddrinfo

int main()
{
  int sockfd;
  struct sockaddr_in serv_addr;

  // 1. Create the socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket creation error");
    return -1;
  }

  // 2. Setup the Server Address (Where we want to go)
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080); // Destination Port

  // Convert string IP "127.0.0.1" to binary
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
  {
    perror("Invalid address/ Address not supported");
    return -1;
  }

  // 3. CONNECT!
  // This blocks the process until the handshake finishes or fails
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("Connection Failed");
    return -1;
  }

  printf("Connected successfully!\n");

  // now we can use send(sockfd, ...) or recv(sockfd, ...)
  close(sockfd);
  return 0;
}

// CHANGE THESE TO TEST GOOGLE
// #define HOST "google.com"
// #define PORT "80"

// #define HOST "localhost"
// #define PORT "8080"

// If we want to use getaddrinfo() then we can go with it in shoip.c file and define host and port and pass that as an arguments we can also set it to connect it with google...