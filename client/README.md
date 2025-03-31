# TinyMQ Client

A simple client implementation for the TinyMQ protocol.

## Overview

This client allows you to connect to a TinyMQ broker, subscribe to topics, publish messages, and receive messages from topics you're subscribed to.

## Features

- Connect to a TinyMQ broker
- Subscribe to topics with callback handlers
- Unsubscribe from topics
- Publish messages to topics
- Interactive command-line interface for testing

## Building

### Prerequisites

- C++17 compatible compiler
- Boost libraries (system and thread components)
- CMake (3.10 or higher)

### Compilation

From the client directory:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Running the Client

```bash
./tinymq_client [client_id]
```

If no client_id is provided, it defaults to "test_client".

### Interactive Commands

Once connected, you can use the following commands:

- `pub <topic> <message>` - Publish a message to a topic
- `sub <topic>` - Subscribe to a topic
- `unsub <topic>` - Unsubscribe from a topic
- `exit` - Exit the client

### Programmatic Usage

You can also use the client in your own code:

```cpp
#include "client.h"

// Create client with ID, host, and port
tinymq::client::Client client("my_client", "localhost", 1505);

// Connect to broker
if (client.connect()) {
    // Subscribe to a topic
    client.subscribe("example/topic", [](const std::string& topic, const std::vector<uint8_t>& message) {
        std::string msg_str(message.begin(), message.end());
        std::cout << "Received: " << msg_str << " on topic: " << topic << std::endl;
    });
    
    // Publish to a topic
    client.publish("example/topic", "Hello, TinyMQ!");
    
    // Process any incoming messages
    client.poll();
    
    // Unsubscribe when done
    client.unsubscribe("example/topic");
    
    // Disconnect
    client.disconnect();
}
```

## Notes

The client connects to `localhost:1505` by default, which is the default address and port of the TinyMQ broker. These values can be changed when creating the client instance if needed. 