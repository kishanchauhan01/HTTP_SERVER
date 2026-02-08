## Socket programming

* Network programming in C used to be messy, requiring different code for IPv4 and IPv6. The struct addrinfo and getaddrinfo() function were introduced to solve this by making your code protocol-independent.

## 1. The Data Structure: struct addrinfo

1. The Data Structure: struct addrinfo
   Think of struct addrinfo as a flexible container that holds everything you need to create a socket and connect to a host.

* Crucially, because a single hostname (like google.com) can have multiple IP addresses (both IPv4 and IPv6) and support multiple protocols, getaddrinfo() returns a linked list of these structures.

```c
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET (IPv4), AF_INET6 (IPv6), AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM (TCP), SOCK_DGRAM (UDP)
    int              ai_protocol;  // IPPROTO_TCP, IPPROTO_UDP
    socklen_t        ai_addrlen;   // Length of the socket address below
    struct sockaddr *ai_addr;      // Pointer to the struct containing IP & Port
    char            *ai_canonname; // Canonical name of the host
    struct addrinfo *ai_next;      // Pointer to the next item in the list
};
```

### Key Fields to Know:

* ai_family: Allows you to request only IPv4, only IPv6, or "Unspecified" (AF_UNSPEC) to get both.

* ai_socktype: Specifies if you want a stream (TCP) or datagram (UDP) socket.

* ai_addr: This is the most important part. It points to a generic struct sockaddr that holds the actual raw IP address and port data required by system calls like connect() or bind().

* ai_next: Points to the next result. You usually loop through this list until you successfully connect.

## 2. The Function: getaddrinfo()

* This function performs the heavy lifting. It does the DNS lookup (converting "https://www.google.com/url?sa=E&source=gmail&q=google.com" to an IP) and service lookup (converting "http" to port 80), and fills out the linked list of addrinfo structures for you.

* ®️ The Signature:

```c
int getaddrinfo(const char *node,
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

### The Arguments:

1. node: The hostname (e.g., "www.example.com") or an IP string (e.g., "192.168.1.1"). If you are creating a server to listen for connections, this can be NULL.

2. service: The service name (e.g., "http", "ftp") or a decimal port number string (e.g., "80", "8080").

3. hints: A pointer to a struct addrinfo that you fill out beforehand to filter results. For example, if you only want TCP sockets, you set hints.ai_socktype = SOCK_STREAM.

4. res: A double pointer. getaddrinfo will allocate a linked list of results and point \*res to the first one.

## 3. How to Use Them (The Workflow)

* The standard workflow for writing a modern network application involves 4 specific steps.

### Step 1: Set up the "Hints"

* You must tell the system what kind of addresses you are looking for.

```c
struct addrinfo hints, *res;

memset(&hints, 0, sizeof hints); // Make sure the struct is empty
hints.ai_family = AF_UNSPEC;     // Don't care if IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me (only for servers)
```

### Step 2: Call the function

* Call getaddrinfo and check for errors.

```c
int status;
if ((status = getaddrinfo("www.example.com", "80", &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}
```

### Step 3: Iterate and Connect

* The res variable now points to a linked list. One hostname might return an IPv6 address first, then an IPv4 address. You should loop through them until one works.

```c
struct addrinfo *p;
int sockfd;

// Loop through all the results and connect to the first we can
for(p = res; p != NULL; p = p->ai_next) {
    // 1. Create the socket
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        continue; // Try the next one
    }

    // 2. Try to connect
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        continue; // Try the next one
    }

    break; // If we get here, we successfully connected!
}
```

### Step 4: Clean up

* Since getaddrinfo allocates memory for that linked list, you must free it to avoid memory leaks.

```c
freeaddrinfo(res); // Free the linked list
```

```c
/*
** showip.c
**
** show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: showip hostname\n");
	    return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // Either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:\n\n", argv[1]);

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(res); // free the linked list

	return 0;
}
```

## 💗 The Relationship: sockaddr vs sockaddr_in

* Think of struct sockaddr as a `Generic Connector` (like a universal power adapter) and struct sockaddr_in as a `Specific Plug` (like a US 3-prong plug).
    - struct sockaddr: The `Generic structure`. The socket functions (like connect, bind, accept) are designed to take a pointer to this structure. They don't know if you are using IPv4, IPv6, or Bluetooth. They just want a "socket address."

    - struct sockaddr_in: The `IPv4-Specific` structure. This is what you actually fill out in your code when using standard internet (IPv4). It has fields specifically for IP addresses and Ports.

### Why do we need two?

* C is strictly typed. The `connect()` function needs to define some argument type. It can't say "accept anything." So, the developers defined a generic type (`sockaddr`).

* However, it is very hard to fill out a `sockaddr` directly because the IP address and port are smashed together into a single `sa_data` array.

* So, you use the "easy" struct (`sockaddr_in`) to fill in your data, and then cast it to the "generic" struct (`sockaddr`) when you call the function.

* 📂 The Structures Side-by-Side:

```c
// The Generic One (Hard to use directly)
struct sockaddr {
    unsigned short    sa_family;    // Address family (e.g., AF_INET)
    char              sa_data[14];  // 14 bytes of protocol address
};

// The IPv4 Specific One (Easy to use)
struct sockaddr_in {
    short int          sin_family;  // Address family (AF_INET)
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Padding to make it the same size as sockaddr
};
```

### The "Magic" of Memory Layout

* Notice that both structures start with the exact same data type: family.
    - When you pass `sockaddr_in` cast as `sockaddr`, the system looks at the first 2 bytes (`family`).

    - If it sees `AF_INET`, it knows, "Ah, the rest of this data is actually organized as a sockaddr_in," and it reads the port and IP correctly.

The Casting Syntax: This is why you see this cast in every network program:

```c
struct sockaddr_in myaddr;
// ... fill myaddr with data ...

// CASTING HAPPENS HERE:
connect(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
```

## socket() function

* The `socket()` function is the foundation of network programming in C. It creates an "endpoint" for communication and returns a `file descriptor (an integer ID)` that you use in all subsequent network calls (like connect, bind, listen, or send).

* The Signature:

```c
int socket(int domain, int type, int protocol);
```

* Here is a breakdown of the three arguments:

### 1) domain (The Family)

* This tells the system which protocol family (or address family) you want to use. It defines the format of the addresses.
    - `AF_INET`: IPv4 (Internet Protocol v4). This is the most common (e.g., 192.168.1.1).

    - `AF_INET6`: IPv6 (Internet Protocol v6). The modern standard (e.g., 2001:0db8::).

    - `AF_UNIX` (or `AF_LOCAL`): `Local communication` on the same machine (Unix Domain Sockets). Used for efficient process-to-process talk without using the network card.

* Note: "AF" stands for `Address Family`. You might sometimes see "PF" (`Protocol Family`), like PF*INET. In modern code, AF_INET and PF_INET are practically interchangeable, but `AF*` is preferred for the domain argument.

### 2) type (The Style of Communication)

* This determines how `data is transferred`.
    - `SOCK_STREAM` (TCP): Provides a sequenced, reliable, two-way, connection-based byte stream.

    - `SOCK_DGRAM` (UDP): Supports datagrams (connectionless, unreliable messages of a fixed maximum length).

    - `SOCK_RAW`: Provides raw network protocol access (used for advanced things like ping tools or sniffing).

### 3) protocol ( The Specific Protocol)

* This specifies a particular protocol to be used with the socket.
    - Usually `0`: In 99% of cases, you set this to `0`. This tells the system to "choose the `default protocol` for the given domain and type."
        - If you choose AF_INET + SOCK_STREAM, the system knows you mean `TCP`.

        - If you choose AF_INET + SOCK_DGRAM, the system knows you mean `UDP`.

    - Explicit values: You can explicitly pass `IPPROTO_TCP` or `IPPROTO_UDP`, but it is rarely necessary unless you are using a non-standard protocol combination.

* Return Value
    - `Success`: Returns a `non-negative` integer (the File Descriptor). You store this in a variable (usually named `sockfd` or `listenfd`).

    - `Error`: Returns `-1`. The specific error code is stored in the global variable `errno` (e.g., `EACCES` if you don't have permission).

## 🥗 bind()—What port am I on?

* Once you have a socket, you might have to associate that socket with a port on your local machine. (This is commonly done if you’re going to listen() for incoming connections on a specific port—multiplayer network games do this when they tell you to “connect to 192.168.5.10 port 3490”.) The port number is used by the kernel to match an incoming packet to a certain process’s socket descriptor. If you’re going to only be doing a connect() (because you’re the client, not the server), this is probably unnecessary. Read it anyway, just for kicks.

* Here is the synopsis for the bind() system call:

```c
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

* `What it does`

* It associates the socket (which currently just exists as a file descriptor) with a specific Local IP `Address` and `Local Port Number` on your machine.
    - `Servers` use this so clients know where to find them (e.g., "Listen on Port 80").

    - `Clients` usually skip this. The OS will automatically assign them a random unused port when they try to connect.

### The Arguments Breakdown

1. sockfd **`(The Socket)`**
    - This is the integer File Descriptor you got returned from the `socket()` call. It tells the system which socket you want to configure.

2) my_addr **`(The Address)`**
    - This is a pointer to the `struct sockaddr` containing the IP and Port you want to lock down.

    - `For IP`: You usually use `INADDR_ANY`. This tells the OS, "Bind to all available interfaces." (e.g., if your server has WiFi and Ethernet, listen on both).

    - `For Port`: You specify the port (e.g., 80 for HTTP, 443 for HTTPS, or 3490 for your custom app).

    - `Crucial Note`: As discussed previously, you actually fill out a struct `sockaddr_in` and cast it to struct `sockaddr*` here.

3) addrlen **`(The Size)`**
    - The size of the address structure. You simply pass sizeof(**struct sockaddr_in)**. This prevents the kernel from reading too much or too little memory.

### The Return Value

* `0`: Success! The socket is now sitting on that port.

* `-1`: Error. (Check `errno`).

* **The Most Common Error: "Address already in use"** If you run your server, stop it, and try to restart it immediately,  `bind()` often fails with this error.
    - Why? The OS keeps the port "reserved" for a few seconds after a connection closes to clean up stray packets.

    - The Fix: You can use `setsockopt()` to allow reuse of the address (I can show you this if needed).

### Code example

```c
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
```

## 🛜 connect()—Hey, you!

* Let’s just pretend for a few minutes that you’re a telnet application. Your user commands you (just like in the movie TRON) to get a socket file descriptor. You comply and call socket(). Next, the user tells you to connect to “10.12.110.57” on port “23” (the standard telnet port). Yow! What do you do now?

* The `connect()` function is the client-side counterpart to bind(). If socket() creates the telephone,  `connect()` is the act of dialing the number to reach a server.

* The Signature

```c
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
```

* sockfd is our friendly neighborhood socket file descriptor, as returned by the socket() call, serv_addr is a struct sockaddr containing the destination port and IP address, and addrlen is the length in bytes of the server address structure.

* All of this information can be gleaned from the results of the getaddrinfo() call, which rocks.

### The Arguments

1. `sockfd` (The Caller): The file descriptor of your socket (the client's socket) that you created earlier with` socket()`.

2. `serv_addr` (The Destination): A pointer to a `struct sockaddr` containing the IP address and Port of the server you want to talk to.
    - Note: Even though this argument is called `serv_addr`, you usually fill out a `struct sockaddr_in` (for IPv4) and cast it, just like you did with `bind`.

3. `addrlen` (The Size): The size of the server address structure (`sizeof(struct sockaddr)`).

#### When you call `connect()` , the kernel doesn't just "set a variable." It initiates actual `network traffic` .

**1) The TCP 3-Way Handshake**

* If you are using TCP (SOCK_STREAM), calling connect() triggers the famous "Three-Way Handshake."
    1.  SYN: Your kernel sends a "Hello, I want to connect" packet (SYN) to the server.

    2.  SYN-ACK: Your kernel blocks (pauses your program) and waits. If the server is listening, it sends back a "Received, let's talk" packet (SYN-ACK).

    3.  ACK: Your kernel sends a final "Okay, connection confirmed" packet (ACK).

* Only after step 3 does the connect() function` return 0` (Success). If step 2 never happens (`timeout`) or the server says "No" (`RST`), the function `returns -1`.

**2. The "Implicit Bind" (Important!)**

* You might notice client code rarely calls `bind()`.
    - Server: Must use `bind()` because it needs a fixed port (e.g., 80) so people can find it.

    - Client: Doesn't care which port it uses, as long as the connection works.

* When you call `connect()`, if you haven't bound a port yet, the operating system automatically checks your local interfaces and assigns you:
    1.  A random, unused Ephemeral Port (usually between 1024 and 65535).

    2.  The correct local IP address to reach the destination.

### Return Values & Common Errors

* `0`: Success. The line is open. You can now use `send()` and `recv()` (or `write`/`read`) on the socket.

* `-1`: Error. The global `errno` variable explains why.

**Common Errors:**

1. `ECONNREFUSED` (Connection Refused):
    - Meaning: You reached the computer, but no program is listening on that specific port.

    - Analogy: You called the house, but nobody picked up.

2. `ETIMEDOUT` (Connection Timed Out):
    - Meaning: You sent the SYN packet, but got no reply at all. The server might be down, or a firewall is silently dropping your packets.

3. `ENETUNREACH` (Network Unreachable):
    - Meaning: Your computer doesn't know how to route packets to that IP (e.g., you have no WiFi).

### Code Example: A Simple Client

* Here is how you use it to connect to "localhost" on port 8080.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    // 1. Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 2. Setup the Server Address (Where we want to go)
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // Destination Port

    // Convert string IP "127.0.0.1" to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // 3. CONNECT!
    // This blocks until the handshake finishes or fails
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected successfully!\n");

    // You can now use send(sockfd, ...) or recv(sockfd, ...)
    close(sockfd);
    return 0;
}
```

## 👂️ listen()—Will somebody please call me?

* What if you don’t want to connect to a remote host. Say, just for kicks, that you want to wait for incoming connections and handle them in some way. The process is two step: first you `listen()`, then you `accept()`

* The listen() call is fairly simple, but requires a bit of explanation:

```c
int listen(int sockfd, int backlog);
```

* sockfd is the usual socket file descriptor from the `socket()` system call. backlog is the number of connections **allowed on the incoming queue**. What does that mean? Well, incoming connections are going to wait in this queue until you `accept()` them and this is the limit on how many can queue up. Most systems silently limit this number to about 20; you can probably get away with setting it to 5 or 10.

* Well, as you can probably imagine, we need to call `bind()` before we call `listen()` so that the server is running on a specific port. (You have to be able to tell your buddies which port to connect to!) So if you’re going to be listening for incoming connections, the sequence of system calls you’ll make is:

```c
getaddrinfo();
socket();
bind();
listen();
/* accept() goes here */
```

### The Arguments

1.  `sockfd` (The Socket)
    The file descriptor you created with socket() and subsequently bound to a port with bind().

2.  `backlog` (The Queue Size)
    This is the number of pending connections the operating system will allow to wait in line.

    - **Scenario**: Imagine your server is busy handling a request (maybe writing to a database). While it is busy, three new clients try to connect at the exact same millisecond.

    - **The Queue**: Since your server can't `accept()` them all instantly, the OS keeps them in a <i>**"waiting room"**</i> (the backlog queue).

    - **The Limit**: If the `backlog` is set to 10, and 11 people try to connect simultaneously, the 11th person will get an error (usually `ECONNREFUSED`) because the queue is full.

### <i>What value should backlog be?</i>

* Historically, this was small (like 5 or 10).

* In modern high-performance servers, this is often set much higher (e.g., 128, 512, or even higher) to handle traffic spikes.

* You can often use the constant SOMAXCONN (Socket Max Connections), which sets it to the maximum safe value allowed by your specific operating system (usually 128 or 4096 on Linux).

### 🌊 Deep Dive: Passive vs. Active Sockets

* This is a key concept that `listen()` changes.

* **Active Socket**: When you first create a socket with `socket()`, the kernel assumes it will be used to initiate a connection (like a client calling `connect()`).

* **Passive Socket**: When you call `listen()`, you flip a switch in the kernel. You are telling it, "This socket will never initiate a connection. It will only wait for others to connect to it."

Once you call `listen()` , you can never call `connect()` on that socket. It is now dedicated permanently to waiting.

### Return Values

* `0`: Success. The socket is now in "listening" mode.

* `-1`: Error. (Check errno).
    - `EADDRINUSE`: You tried to listen on a port that another program is already using (common if you forgot to check `bind()` return value).

    - `EBADF`: The `sockfd` is invalid.

### Code Example: The Server Setup Sequence

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd;
    struct sockaddr_in my_addr;

    // 1. SOCKET
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. BIND (Setup the address)
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(8080);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
        perror("bind failed");
        exit(1);
    }

    // 3. LISTEN
    // We allow a backlog of 20 pending connections
    if (listen(sockfd, 20) == -1) {
        perror("listen failed");
        exit(1);
    }

    printf("Server is listening on port 8080...\n");

    // 4. ACCEPT (Wait for the phone to ring)
    // The program will pause here until a client connects

    return 0;`
}
```

## 🉑 accept()—“Thank you for calling port 3490.”

* The `accept()` function is the final step in the server setup.

* If `listen()` is waiting for the phone to ring,  `accept()` is picking up the handset to say "Hello."

* This function is special because it is where the "magic" of handling multiple clients happens.

* **The Signature**

```c
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### The " `Two Socket` " Concept (Crucial!)

* This is the most confusing part for beginners. When `accept()` returns successfully, you suddenly have two sockets.
    1) **The Listening Socket** ( `sockfd` ): This is the one you passed as an argument. It never transfers data. Its only job is to stay at the door and wait for new people. It remains open.

    2) **The Connected Socket** (**The Return Value**): `accept()` creates a **brand new** socket file descriptor specifically for this connection. You use this new socket to `send()` and `recv()` data with the client.

* **Analogy**: Think of a receptionist at a busy office.

    - `The Receptionist` (**sockfd**): Stands at the front door. When a visitor arrives, the receptionist doesn't stop to have a 30-minute meeting with them.

    - `The Meeting Room` (**new_fd**): The receptionist hands the visitor a badge and points them to a private room. The meeting happens in that room, while the receptionist immediately goes back to the door to wait for the next visitor.

### The Arguments

1) `sockfd` (The Receptionist): The original socket that is currently in `listen()` mode.

2) `addr` (The ID Card): This is a pointer to a `struct sockaddr` (usually `sockaddr_in` ).

    - **Input**: You provide an empty struct.

    - **Output**: When `accept()` returns, this struct will be **filled with the Client's IP address and Port**. This is how you know who just called you!

3) `addrlen` (The Size): This is a pointer to an integer.

    - **Must be initialized**: Before calling `accept`, set this to `sizeof(struct sockaddr_in)`.

    - **Value-Result**: `accept` will update this integer to tell you exactly how many bytes of the struct it actually used.

### Deep Dive: Blocking Behavior

By default, `accept()` is a blocking call.

* If the connection queue (**backlog**) is empty, your program simply stops at this line. It freezes.
 
* It sleeps **efficiently** (consuming almost 0% CPU) until a new TCP connection **handshake** is completed.

* The moment a client connects, the OS wakes your program up,  `accept` returns the `new socket ID`, and your code continues.

### Return Values

* **Non-negative Integer**: The File Descriptor for the **new connected socket**. You must save this to talk to the client.

* `-1`: Error. (Check `errno`). 

### code example

```c
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
```

