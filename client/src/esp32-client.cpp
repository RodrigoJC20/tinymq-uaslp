#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include "json.hpp"
#include "client.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

const int port = 12345;

int main() {
    try {
        boost::asio::io_context io;

        // Start TinyMQ client
        tinymq::client::Client client("esp32");
        if (!client.connect()) {
            std::cerr << "Failed to connect to TinyMQ broker." << std::endl;
            return 1;
        }

        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));
        std::cout << "Listening on port " << port << std::endl;

        while (true) {
            tcp::socket socket(io);
            acceptor.accept(socket);
            std::cout << "Client connected." << std::endl;

            boost::asio::streambuf buffer;
            boost::system::error_code ec;

            while (boost::asio::read_until(socket, buffer, "\n", ec)) {
                std::istream is(&buffer);
                std::string line;
                std::getline(is, line);

                try {
                    auto parsed = json::parse(line);
                    std::string topic = parsed["topic"];
                    std::string data = parsed["data"];
                    std::cout << "Publishing [" << topic << "]: " << data << std::endl;
                    client.publish(topic, data);
                } catch (const std::exception& e) {
                    std::cerr << "Invalid JSON: " << e.what() << std::endl;
                }
            }

            // Handle disconnects and other errors
            if (ec == boost::asio::error::eof) {
                std::cout << "Client disconnected gracefully." << std::endl;
            } else if (ec == boost::asio::error::connection_reset) {
                std::cout << "Client disconnected unexpectedly." << std::endl;
            } else if (ec) {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
