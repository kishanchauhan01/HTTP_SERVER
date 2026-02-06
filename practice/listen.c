#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
  int sockfd;
  struct sockaddr_in my_addr;

  // 1. SOCKET
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // 2. BIND (Setup the address)
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(8080);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
  {
    perror("bind failed");
    exit(1);
  }

  // 3. LISTEN
  // We allow a backlog of 20 pending connections
  if (listen(sockfd, 20) == -1)
  {
    perror("listen failed");
    exit(1);
  }

  printf("Server is listening on port 8080...\n");

  // 4. ACCEPT (Wait for the phone to ring)
  // The program will pause here until a client connects
  // accept(sockfd, ...);

  return 0;
}