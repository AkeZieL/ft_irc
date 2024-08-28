#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <iostream>

#include "Client.hpp"

class Client;

class Channel {
	private:
		const std::string _channel_name;
		const Client* _creator;
		std::vector<Client*> _operator;
		std::vector<Client*> _regular_user;

		bool is_operator;
		//Mode
		bool _invite;
		bool _topic;
		std::string _key;
		size_t _limit;

		Channel();
	public:
		Channel(const std::string &channel_name, const std::string& key, Client* creator);
		~Channel();

		bool remove_client(Client* client);

		//getter
		const std::string get_channel_name() const;
		size_t get_limit() const;
		size_t get_client_size() const;
		std::string get_key() const;
		std::vector<std::vector<Client*> > get_client_in_channel() const;
		std::string get_topic() const;

		//setter
		void set_client(Client* client, bool is_operator);
};
