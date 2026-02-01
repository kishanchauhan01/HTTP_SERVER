## Socket programming

- Network programming in C used to be messy, requiring different code for IPv4 and IPv6. The struct addrinfo and getaddrinfo() function were introduced to solve this by making your code protocol-independent.

## 1. The Data Structure: struct addrinfo

1. The Data Structure: struct addrinfo
   Think of struct addrinfo as a flexible container that holds everything you need to create a socket and connect to a host.

- Crucially, because a single hostname (like google.com) can have multiple IP addresses (both IPv4 and IPv6) and support multiple protocols, getaddrinfo() returns a linked list of these structures.

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

- ai_family: Allows you to request only IPv4, only IPv6, or "Unspecified" (AF_UNSPEC) to get both.

- ai_socktype: Specifies if you want a stream (TCP) or datagram (UDP) socket.

- ai_addr: This is the most important part. It points to a generic struct sockaddr that holds the actual raw IP address and port data required by system calls like connect() or bind().

- ai_next: Points to the next result. You usually loop through this list until you successfully connect.

## 2. The Function: getaddrinfo()

- This function performs the heavy lifting. It does the DNS lookup (converting "https://www.google.com/url?sa=E&source=gmail&q=google.com" to an IP) and service lookup (converting "http" to port 80), and fills out the linked list of addrinfo structures for you.

- ®️ The Signature:

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

- The standard workflow for writing a modern network application involves 4 specific steps.

### Step 1: Set up the "Hints"

- You must tell the system what kind of addresses you are looking for.

```c
struct addrinfo hints, *res;

memset(&hints, 0, sizeof hints); // Make sure the struct is empty
hints.ai_family = AF_UNSPEC;     // Don't care if IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me (only for servers)
```

### Step 2: Call the function

- Call getaddrinfo and check for errors.

```c
int status;
if ((status = getaddrinfo("www.example.com", "80", &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}
```

### Step 3: Iterate and Connect

- The res variable now points to a linked list. One hostname might return an IPv6 address first, then an IPv4 address. You should loop through them until one works.

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

- Since getaddrinfo allocates memory for that linked list, you must free it to avoid memory leaks.

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

- Think of struct sockaddr as a `Generic Connector` (like a universal power adapter) and struct sockaddr_in as a `Specific Plug` (like a US 3-prong plug).
    - struct sockaddr: The `Generic structure`. The socket functions (like connect, bind, accept) are designed to take a pointer to this structure. They don't know if you are using IPv4, IPv6, or Bluetooth. They just want a "socket address."

    - struct sockaddr_in: The `IPv4-Specific` structure. This is what you actually fill out in your code when using standard internet (IPv4). It has fields specifically for IP addresses and Ports.

### Why do we need two?

- C is strictly typed. The `connect()` function needs to define some argument type. It can't say "accept anything." So, the developers defined a generic type (`sockaddr`).

- However, it is very hard to fill out a `sockaddr` directly because the IP address and port are smashed together into a single `sa_data` array.

- So, you use the "easy" struct (`sockaddr_in`) to fill in your data, and then cast it to the "generic" struct (`sockaddr`) when you call the function.

- 📂 The Structures Side-by-Side:

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

- Notice that both structures start with the exact same data type: family.

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

- The `socket()` function is the foundation of network programming in C. It creates an "endpoint" for communication and returns a `file descriptor (an integer ID)` that you use in all subsequent network calls (like connect, bind, listen, or send).

- The Signature:

```c
int socket(int domain, int type, int protocol);
```

- Here is a breakdown of the three arguments:

### 1) domain (The Family)

- This tells the system which protocol family (or address family) you want to use. It defines the format of the addresses.

	- `AF_INET`: IPv4 (Internet Protocol v4). This is the most common (e.g., 192.168.1.1).

	- `AF_INET6`: IPv6 (Internet Protocol v6). The modern standard (e.g., 2001:0db8::).

	- `AF_UNIX` (or `AF_LOCAL`): `Local communication` on the same machine (Unix Domain Sockets). Used for efficient process-to-process talk without using the network card.

- Note: "AF" stands for `Address Family`. You might sometimes see "PF" (`Protocol Family`), like PF_INET. In modern code, AF_INET and PF_INET are practically interchangeable, but `AF_` is preferred for the domain argument.

### 2) type (The Style of Communication)

- This determines how `data is transferred`.

	- `SOCK_STREAM` (TCP): Provides a sequenced, reliable, two-way, connection-based byte stream.

	- `SOCK_DGRAM` (UDP): Supports datagrams (connectionless, unreliable messages of a fixed maximum length).

	- `SOCK_RAW`: Provides raw network protocol access (used for advanced things like ping tools or sniffing).

### 3) protocol ( The Specific Protocol)

- This specifies a particular protocol to be used with the socket.

	- Usually `0`: In 99% of cases, you set this to `0`. This tells the system to "choose the `default protocol` for the given domain and type."

		- If you choose AF_INET + SOCK_STREAM, the system knows you mean `TCP`.

		- If you choose AF_INET + SOCK_DGRAM, the system knows you mean `UDP`.

	- Explicit values: You can explicitly pass `IPPROTO_TCP` or `IPPROTO_UDP`, but it is rarely necessary unless you are using a non-standard protocol combination.

- Return Value

	- `Success`: Returns a `non-negative` integer (the File Descriptor). You store this in a variable (usually named `sockfd` or `listenfd`).

	- `Error`: Returns `-1`. The specific error code is stored in the global variable `errno` (e.g., `EACCES` if you don't have permission).


## 🥗 bind()—What port am I on?

- Once you have a socket, you might have to associate that socket with a port on your local machine. (This is commonly done if you’re going to listen() for incoming connections on a specific port—multiplayer network games do this when they tell you to “connect to 192.168.5.10 port 3490”.) The port number is used by the kernel to match an incoming packet to a certain process’s socket descriptor. If you’re going to only be doing a connect() (because you’re the client, not the server), this is probably unnecessary. Read it anyway, just for kicks.

- Here is the synopsis for the bind() system call:

```c
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

- `What it does`

- It associates the socket (which currently just exists as a file descriptor) with a specific Local IP `Address` and `Local Port Number` on your machine.

	- `Servers` use this so clients know where to find them (e.g., "Listen on Port 80").

	- `Clients` usually skip this. The OS will automatically assign them a random unused port when they try to connect.

### The Arguments Breakdown

1) sockfd **`(The Socket)`**

	- This is the integer File Descriptor you got returned from the `socket()` call. It tells the system which socket you want to configure.

2. my_addr **`(The Address)`**

	- This is a pointer to the `struct sockaddr` containing the IP and Port you want to lock down.

	- `For IP`: You usually use `INADDR_ANY`. This tells the OS, "Bind to all available interfaces." (e.g., if your server has WiFi and Ethernet, listen on both).

	- `For Port`: You specify the port (e.g., 80 for HTTP, 443 for HTTPS, or 3490 for your custom app).

	- `Crucial Note`: As discussed previously, you actually fill out a struct `sockaddr_in` and cast it to struct `sockaddr*` here.

3. addrlen **`(The Size)`**

	 - The size of the address structure. You simply pass sizeof(**struct sockaddr_in)**. This prevents the kernel from reading too much or too little memory.

### The Return Value

- `0`: Success! The socket is now sitting on that port.

- `-1`: Error. (Check `errno`).

- **The Most Common Error: "Address already in use"** If you run your server, stop it, and try to restart it immediately, `bind()` often fails with this error.

	- Why? The OS keeps the port "reserved" for a few seconds after a connection closes to clean up stray packets.

	- The Fix: You can use `setsockopt()` to allow reuse of the address (I can show you this if needed).