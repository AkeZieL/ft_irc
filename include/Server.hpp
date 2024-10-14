#pragma once

#include <iostream>
#include <vector>
#include <map>

#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <csignal>

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"

#define MAX_CLIENTS 100

class Parser;
class Client;
class Channel;

class Server {
    private:
        Server();
        Server(const Server &copy);

        std::string _port;
        std::string _passwd;
        int _socket_fd;
        bool _running;
	Parser* _parser;

        std::vector<pollfd> _poll_fd;
        std::map<int, Client*> _clients;
	std::vector<Channel*> _channels;

        void client_connect();
        void client_message(std::vector<pollfd>::iterator it_client);
        int create_socket();
    public:
        Server(std::string port, std::string passwd);
        ~Server();

        static void signal_handler(int signum);
        void start();

        void client_disconnect(int client_fd);

	//Create channel
	Channel* create_channel(Client* client, std::vector<std::string> channel, std::vector<std::string> key, size_t i);

	//getter
	Client* get_client_nickname(const std::string& nickname) const;
	std::string get_password() const;
	Channel* get_channel(const std::string& channel) const;
	std::vector<Channel*> get_all_channel() const;

	//setter
	void set_channels(Channel* channel);
};
