#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>

int main()
{

  int sockfd;
  struct sockaddr_in my_addr;

  // 1. Create the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // 2. Prepare the address struct
  my_addr.sin_family = AF_INET;         // Host byte order
  my_addr.sin_port = htons(8080);       // Short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // Auto-fill with my IP
  memset(&(my_addr.sin_zero), '\0', 8); // Zero the rest of the struct

  // 3. BIND!
  // Link the "sockfd" to the information in "my_addr"
  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("bind connection error"); // Print error if port 8080 is busy
    exit(1);
  }

  // Now the socket is bound to port 8080!
  // Next steps: listen() -> accept()

  return 0;
}