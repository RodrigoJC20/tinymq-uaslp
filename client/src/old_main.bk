#include "client.h"
#include "terminal_ui.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <unordered_set>

tinymq::client::Client* g_client = nullptr;

void signal_handler(int signal) {
    tinymq::ui::print_message("Signal", "Received signal " + std::to_string(signal) + ", shutting down...", 
                             tinymq::ui::MessageType::WARNING);
    if (g_client) {
        g_client->disconnect();
        exit(0);
    }
}

std::string bytes_to_string(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

void print_help() {
    std::cout << tinymq::ui::CYAN << "\nCommands:" << tinymq::ui::RESET << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "pub <topic> <message>" << tinymq::ui::RESET << " - Publish a message to a topic" << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "sub <topic>" << tinymq::ui::RESET << " - Subscribe to a topic" << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "unsub <topic>" << tinymq::ui::RESET << " - Unsubscribe from a topic" << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "topics" << tinymq::ui::RESET << " - List all subscribed topics" << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "help" << tinymq::ui::RESET << " - Show this help message" << std::endl;
    std::cout << "  " << tinymq::ui::GREEN << "exit" << tinymq::ui::RESET << " - Exit the client\n" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string client_id = "test_client";
    if (argc > 1) {
        client_id = argv[1];
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        tinymq::ui::print_header("TinyMQ Client");
        
        tinymq::client::Client client(client_id);
        g_client = &client;
        
        tinymq::ui::print_message("Client", "Starting client with ID: " + client_id, tinymq::ui::MessageType::INFO);
        
        if (!client.connect()) {
            tinymq::ui::print_message("Client", "Failed to connect to broker", tinymq::ui::MessageType::ERROR);
            return 1;
        }
        
        client.subscribe("test", [](const std::string& topic, const std::vector<uint8_t>& message) {});
        
        std::unordered_set<std::string> subscribed_topics = {"test"};
        
        print_help();
        
        std::string command;
        
        while (true) {
            std::cout << tinymq::ui::BOLD << tinymq::ui::BLUE << "> " << tinymq::ui::RESET;
            std::getline(std::cin, command);
            
            if (command.empty()) {
                continue;
            }
            
            std::string cmd;
            std::string arg1;
            std::string arg2;
            
            size_t pos = command.find(' ');
            if (pos != std::string::npos) {
                cmd = command.substr(0, pos);
                std::string remainder = command.substr(pos + 1);
                
                size_t pos2 = remainder.find(' ');
                if (pos2 != std::string::npos) {
                    arg1 = remainder.substr(0, pos2);
                    arg2 = remainder.substr(pos2 + 1);
                } else {
                    arg1 = remainder;
                }
            } else {
                cmd = command;
            }
            
            if (cmd == "exit") {
                break;
            } else if (cmd == "help") {
                print_help();
            } else if (cmd == "topics") {
                tinymq::ui::print_message("Client", "Subscribed topics:", tinymq::ui::MessageType::INFO);
                if (subscribed_topics.empty()) {
                    std::cout << "  No subscriptions" << std::endl;
                } else {
                    for (const auto& topic : subscribed_topics) {
                        std::cout << "  - " << topic << std::endl;
                    }
                }
            } else if (cmd == "pub" && !arg1.empty() && !arg2.empty()) {
                client.publish(arg1, arg2);
            } else if (cmd == "sub" && !arg1.empty()) {
                if (client.subscribe(arg1, [](const std::string& topic, const std::vector<uint8_t>& message) {})) {
                    subscribed_topics.insert(arg1);
                }
            } else if (cmd == "unsub" && !arg1.empty()) {
                if (client.unsubscribe(arg1)) {
                    subscribed_topics.erase(arg1);
                }
            } else {
                tinymq::ui::print_message("Client", "Invalid command. Type 'help' for available commands.", 
                                        tinymq::ui::MessageType::WARNING);
            }
            
            client.poll();
        }
        
        client.disconnect();
        g_client = nullptr;
        
        return 0;
    } catch (const std::exception& e) {
        tinymq::ui::print_message("Client", "Exception: " + std::string(e.what()), tinymq::ui::MessageType::ERROR);
        return 1;
    }
} 