# TinyMQ

A lightweight and simple implementation of an MQTT-like protocol in C++.

## Overview

TinyMQ is a minimal implementation of a publish-subscribe messaging protocol inspired by MQTT. It focuses on simplicity while providing the core functionality needed for a messaging broker:

- Basic connection handling
- Topic subscription and unsubscription
- Message publishing

## Features

- Lightweight packet structure
- Simple broker implementation using Boost ASIO
- Thread pool for handling multiple connections concurrently
- Support for basic MQTT-like operations
- Client implementation for testing and application integration

## Components

- **Broker**: The server component that handles connections and routes messages
- **Client**: A library and command-line application for connecting to the broker

## Packet Types

TinyMQ supports the following packet types:

- `CONN` (0x01): Initial connection request
- `CONNACK` (0x02): Connection acknowledgment
- `PUB` (0x03): Publish message
- `PUBACK` (0x04): Publish acknowledgment
- `SUB` (0x05): Subscribe to topic
- `SUBACK` (0x06): Subscribe acknowledgment
- `UNSUB` (0x07): Unsubscribe from topic
- `UNSUBACK` (0x08): Unsubscribe acknowledgment

## Packet Structure

Every TinyMQ packet consists of:

- Packet Type (1 byte): Identifies the type of packet
- Flags (1 byte): Reserved for future extensions
- Payload Length (2 bytes): Length of the payload data
- Payload: Variable length data depending on packet type

## Building

### Prerequisites

- C++17 compatible compiler
- Boost libraries (system and thread components)
- CMake (3.10 or higher)

### Compilation

#### Building the Broker

```bash
cd tinymq
mkdir build
cd build
cmake ..
make
```

#### Building the Client

```bash
cd tinymq/client
mkdir build
cd build
cmake ..
make
```

## Usage

### Running the Broker

```bash
./tinymq_broker [--port PORT] [--threads NUM_THREADS]
```

Options:
- `--port PORT`: Set the port number (default: 1505)
- `--threads N`: Set thread pool size (default: 4)

### Running the Client

```bash
./tinymq_client [client_id]
```

Client commands:
- `pub <topic> <message>` - Publish a message to a topic
- `sub <topic>` - Subscribe to a topic
- `unsub <topic>` - Unsubscribe from a topic
- `exit` - Exit the client

## Future Work

- Add QoS levels
- Add persistent sessions
- Add authentication and security features
- Improve error handling
- Add wildcard topic subscriptions
- Support for retained messages

## License

This project is open source and available under the MIT license. 

## Project Structure

```
tinymq/
├── CMakeLists.txt          # Broker CMake file
├── README.md               # Main project README
├── build.sh                # Build script for both components
├── client/                 # Client directory
│   ├── CMakeLists.txt      # Client CMake file
│   ├── README.md           # Client README
│   └── src/                # Client source files
│       ├── client.cpp      # Client implementation
│       ├── client.h        # Client header
│       └── main.cpp        # Client executable
└── src/                   # Broker source files
    ├── broker.cpp         # Broker implementation
    ├── broker.h           # Broker header
    ├── main.cpp           # Broker executable
    ├── packet.cpp         # Packet implementation
    ├── packet.h           # Packet header
    ├── session.cpp        # Session implementation
    └── session.h          # Session header
``` 