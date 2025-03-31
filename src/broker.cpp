#include "broker.h"
#include "session.h"
#include "terminal_ui.h"
#include <algorithm>
#include <iostream>

namespace tinymq {

Broker::Broker(uint16_t port, size_t thread_pool_size)
    : io_context_(),
      acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      thread_pool_size_(thread_pool_size),
      running_(false) {
}

Broker::~Broker() {
    stop();
}

void Broker::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    accept_connections();
    
    threads_.reserve(thread_pool_size_);
    for (size_t i = 0; i < thread_pool_size_; ++i) {
        threads_.emplace_back([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                ui::print_message("Thread", "Exception: " + std::string(e.what()), ui::MessageType::ERROR);
            }
        });
    }
    
    ui::print_message("Broker", "Started on port " + std::to_string(acceptor_.local_endpoint().port()) + 
                     " with " + std::to_string(thread_pool_size_) + " threads", 
                     ui::MessageType::SUCCESS);
}

void Broker::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    acceptor_.close();
    
    io_context_.stop();
    
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.clear();
    }
    
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        topic_subscribers_.clear();
    }
    
    threads_.clear();
    
    ui::print_message("Broker", "Stopped", ui::MessageType::INFO);
}

void Broker::accept_connections() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(socket), *this);
                ui::print_message("Broker", "New connection from " + 
                                session->remote_endpoint(), ui::MessageType::INCOMING);
                session->start();
            } else {
                ui::print_message("Broker", "Accept error: " + ec.message(), ui::MessageType::ERROR);
            }
            
            if (running_) {
                accept_connections();
            }
        });
}

void Broker::register_session(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    const auto& client_id = session->client_id();
    
    auto it = sessions_.find(client_id);
    if (it != sessions_.end()) {
        std::shared_ptr<Session> old_session = it->second;
        
        ui::print_message("Broker", "Client ID already in use, disconnecting old session: " + client_id, 
                        ui::MessageType::WARNING);
        
        {
            std::lock_guard<std::mutex> topics_lock(topics_mutex_);
            for (auto& topic_entry : topic_subscribers_) {
                auto& subscribers = topic_entry.second;
                subscribers.erase(
                    std::remove_if(
                        subscribers.begin(),
                        subscribers.end(),
                        [&old_session](const std::shared_ptr<Session>& s) {
                            return s == old_session;
                        }),
                    subscribers.end());
            }
        }
        
        it->second.reset();
    }
    
    sessions_[client_id] = session;
    ui::print_message("Broker", "Session registered: " + client_id, ui::MessageType::SUCCESS);
}

void Broker::remove_session(std::shared_ptr<Session> session) {
    const auto& client_id = session->client_id();
    
    if (client_id.empty()) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.erase(client_id);
    }
    
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        for (auto& topic_entry : topic_subscribers_) {
            auto& subscribers = topic_entry.second;
            subscribers.erase(
                std::remove_if(
                    subscribers.begin(),
                    subscribers.end(),
                    [&session](const std::shared_ptr<Session>& s) {
                        return s == session;
                    }),
                subscribers.end());
        }
    }
    
    ui::print_message("Broker", "Session removed: " + client_id, ui::MessageType::INFO);
}

void Broker::subscribe(std::shared_ptr<Session> session, const std::string& topic) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    
    auto& subscribers = topic_subscribers_[topic];
    if (std::find(subscribers.begin(), subscribers.end(), session) == subscribers.end()) {
        subscribers.push_back(session);
        ui::print_message("Topic", "Client " + session->client_id() + 
                        " subscribed to topic: " + topic, ui::MessageType::INFO);
    }
}

void Broker::unsubscribe(std::shared_ptr<Session> session, const std::string& topic) {
    std::lock_guard<std::mutex> lock(topics_mutex_);
    
    // Find the topic
    auto it = topic_subscribers_.find(topic);
    if (it != topic_subscribers_.end()) {
        auto& subscribers = it->second;
        subscribers.erase(
            std::remove(subscribers.begin(), subscribers.end(), session),
            subscribers.end());
        
        ui::print_message("Topic", "Client " + session->client_id() + 
                        " unsubscribed from topic: " + topic, ui::MessageType::INFO);
        
        if (subscribers.empty()) {
            topic_subscribers_.erase(it);
        }
    }
}

void Broker::publish(const std::string& topic, const std::vector<uint8_t>& message) {
    std::vector<std::shared_ptr<Session>> subscribers;
    
    {
        std::lock_guard<std::mutex> lock(topics_mutex_);
        auto it = topic_subscribers_.find(topic);
        if (it != topic_subscribers_.end()) {
            subscribers = it->second;
        }
    }
    
    if (subscribers.empty()) {
        ui::print_message("Topic", "No subscribers for topic: " + topic, ui::MessageType::INFO);
        return;
    }
    
    ui::print_message("Topic", "Publishing to " + std::to_string(subscribers.size()) + 
                   " subscribers on topic: " + topic, ui::MessageType::OUTGOING);
    
    std::vector<uint8_t> payload;
    
    payload.push_back(static_cast<uint8_t>(topic.size()));
    
    payload.insert(payload.end(), topic.begin(), topic.end());
    
    payload.insert(payload.end(), message.begin(), message.end());
    
    Packet packet(PacketType::PUB, 0, payload);
    
    for (auto& subscriber : subscribers) {
        subscriber->send_packet(packet);
    }
}

} // namespace tinymq 