#!/bin/bash

# Exit on error
set -e

# Build the broker
echo "Building TinyMQ Broker..."
mkdir -p build
cd build
cmake ..
make
chmod +x tinymq_broker
cd ..

# Build the client
echo "Building TinyMQ Client..."
mkdir -p client/build
cd client/build
cmake ..
make
chmod +x tinymq_client
cd ../..

echo "Build completed successfully!"
echo ""
echo "To run the broker:  ./build/tinymq_broker"
echo "To run the client:  ./client/build/tinymq_client [client_id]"
echo "" 