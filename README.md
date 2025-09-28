# Reti di Calcolatori (Computer Networks)

**Author**: Francesco Ferrillo  
**Year**: University Project  
**Description**: A collection of assignments, exercises, and network applications developed for the **Computer Networks** course. The repository includes socket programming examples, networking protocols exercises, and documentation related to network architectures and communication models.

## Table of Contents

- [Overview](#overview)  
- [Technologies](#technologies)   
- [Features / Modules](#features--modules)  
- [Getting Started](#getting-started)  
- [Running Network Programs](#running-network-programs)  
- [How to Contribute](#how-to-contribute)    

## Overview

This repository serves as a **portfolio of coursework and exercises** completed during the *“Reti di Calcolatori”* (Computer Networks) class. It demonstrates fundamental concepts of computer networking and practical programming of client–server applications using sockets.

The focus is on:
- Understanding network layers and protocols  
- Implementing basic communication mechanisms  
- Writing and running simple client-server programs  
- Working with TCP and UDP connections  
- Analyzing network behavior

## Technologies

- **C / C++** — for socket programming and protocol implementation  
- **Networking APIs** — (e.g., Berkeley sockets)  
- **Shell / Bash** — for running and testing network applications  
- (Optionally) Wireshark or similar tools for packet inspection

## Features / Modules

Typical modules or exercises in this repository cover:

- Client–Server communication using sockets  
- TCP and UDP implementations  
- Request–Response handling  
- Multi-client handling (e.g., `select()`, `fork()`)  
- Packet analysis and network debugging  
- Basic protocol design exercises

## Getting Started

To explore or run the code on your machine:

1. **Clone the repository**
   ```bash
   git clone https://github.com/FrancescoFerrillo/Reti-di-Calcolatori.git
   cd Reti-di-Calcolatori
   ```

2. **Browse the folders**  
   - Check `Esercizi/` for lab exercises  
   - Check `Programmi/` for runnable network programs

## Running Network Programs

Compile and run a server and client in separate terminals.  
For example, if you have `server.c` and `client.c`:

```bash
# Compile
gcc -o server server.c
gcc -o client client.c

# Run the server (Terminal 1)
./server

# Run the client (Terminal 2)
./client
```

Depending on the exercise, you might use specific ports or IP addresses.  
For UDP examples, simply replace the socket type in the code.

## How to Contribute

Although this is an academic project, you can contribute by:

1. Forking the repository  
2. Creating a branch (`feature/your-improvement`)  
3. Committing your changes  
4. Opening a Pull Request with a clear description
