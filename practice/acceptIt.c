#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    int sockfd, new_conn_fd;
    struct sockaddr_in my_addr, client_addr;
    socklen_t client_len;
    char client_ip[INET_ADDRSTRLEN];

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

    printf("Waiting for connection... \n");

    while (1)
    { // Infinite loop to keep server running

        client_len = sizeof(client_addr); // Reset size for every new client

        // Block HERE until someone connects
        new_conn_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);

        if (new_conn_fd == -1)
        {
            perror("accept failed");
            continue;
        }

        // Identify the client (Optional)
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Accepted connection from %s\n", client_ip);

        // --- CONVERSATION PHASE ---
        send(new_conn_fd, "Hello!\n", 7, 0);

        // --- GOODBYE PHASE ---
        // Close the specific conversation socket
        close(new_conn_fd);

        // Loop back up to accept(sockfd) again!
    }

    return 0;
}