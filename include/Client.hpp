#pragma once

#include <string>

#include "Channel.hpp"

#ifndef MAX_CHANNEL
# define MAX_CHANNEL 10
#endif

class Channel;

class Client {
    private:
        Client();
        
	int _client_fd;
	bool _password;
	bool _connected;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::vector<Channel*> _channels;

    public:
        Client(int client_fd);
	Client &operator=(const Client &copy);
	Client(const Client& copy);
        ~Client();

	void join_channel(Client* client, std::string key, Channel* channel);
	void part_channel(Client* client, Channel* channel, std::string message);

	//getter
        int get_client_fd() const;
	std::string get_nickname() const;
	std::string get_username() const;
	std::string get_realname() const;
	bool get_password() const;
	bool get_connected() const;
	size_t get_nbr_channel() const;
	std::vector<Channel*> get_all_channel() const;

	//setter
	void set_nickname(const std::string& nickname);
	void set_realname(const std::string& realname);
	void set_username(const std::string& username);
	void set_password(const bool& password);
	void set_connected(const bool& connected);
	void set_channels(Channel* channel);
};
