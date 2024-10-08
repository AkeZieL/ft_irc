#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <limits>

#include "Client.hpp"
#include "Server.hpp"

class Client;
class Server;

class Channel {
	private:
		Server* _serv;

		const std::string _channel_name;
		const Client* _creator;
		std::vector<Client*> _operator;
		std::vector<Client*> _regular_user;

		std::string _topic;
		//Mode
		bool _invite;
		bool _bool_topic;
		std::string _key;
		size_t _limit;

		Channel();

		size_t string_to_size_t(const std::string& str) const;
	public:
		Channel(const std::string &channel_name, const std::string& key, Client* creator, Server* serv);
		~Channel();

		bool remove_client(Client* client);
		void add_mode(Client* client, std::string command, std::vector<std::string> command_arg);
		void remove_mode(Client* client, std::string command, std::vector<std::string> command_arg);
		void kick(Client* client, Client* client_to_kick, std::string comment);
		bool client_is_in_channel(Client* client) const;
		void mode_states(Client* client, std::string command) const;
		std::string size_t_to_string(const size_t& limit) const;

		//getter
		const std::string get_channel_name() const;
		size_t get_limit() const;
		size_t get_client_size() const;
		std::string get_key() const;
		std::vector<std::vector<Client*> > get_client_in_channel() const;
		std::string get_topic() const;
		bool client_is_operator(Client* client) const;

		//setter
		void set_nickname(Client* client, std::string nickname);
		void set_client(Client* client, bool is_operator);
		void set_topic(Client* client, std::string topic);
};
