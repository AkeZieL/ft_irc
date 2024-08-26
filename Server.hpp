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

#include "Client.hpp"

#define MAX_CLIENTS 100

class Parser;

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

        void client_connect();
        void client_disconnect(int client_fd);
        void client_message(int client_fd);
        int create_socket();
    public:
        Server(std::string port, std::string passwd);
        ~Server();

        void start();
	Client* get_client_nickname(std::string nickname) const;
	std::string get_password() const;
};
