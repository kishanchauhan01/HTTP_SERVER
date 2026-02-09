# HTTP Server (Learning Project)

A low-level HTTP server project written in **C++**, built from scratch to understand:

* TCP socket programming
* Server architecture design
* Client connection handling
* Layered networking design

⚠️ Currently this project only implements the **TCP server foundation**.  
HTTP protocol support will be added later.

---

## Current Features

* TCP listening server
* Client connection handling
* Modular architecture
* Socket abstraction layer
* Designed for multi-client support (work in progress)

---

## Project Structure

HTTP_SERVER/
│
├── src/
│ ├── core/
│ │ ├── server.cpp
│ │ ├── server.hpp
│ │ ├── socket.cpp
│ │ ├── socket.hpp
│ │
│ ├── network/
│ │ ├── tcp_connection.cpp
│ │ ├── tcp_connection.hpp
│
├── build/
│ └── (compiled binaries)
│
├── practice/
│ └── experimental / learning code
│
├── Makefile
├── .clang-format
├── README.md
├── notes.md
├── todo.md
└── LICENSE

---

## Folder Responsibilities

### `core/`

Server lifecycle and low-level socket handling.

### `network/`

Handles communication with connected clients.

### `practice/`

Contains experimental and learning implementations.

### `build/`

Stores compiled output.

---

## Goals

* Understand networking from OS level
* Build HTTP server without frameworks
* Practice clean layered architecture
* Learn multi-client and concurrent server design

---

## Future Roadmap

* Multi-client support
* Thread-per-client model
* HTTP request parsing
* HTTP response generation
* Static file serving
* Logging system

---

## Learning Purpose

This project is intentionally written without external networking libraries to gain deeper understanding of:

* POSIX sockets
* TCP lifecycle
* Server design patterns
