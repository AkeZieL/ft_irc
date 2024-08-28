#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "Server.hpp"
#include "Channel.hpp"


class Parser;

typedef void (Parser::*commands_ptr)(Client*, const std::vector<std::string>&);

class Parser {
	private:
		Parser();

		std::map<std::string, commands_ptr> _commands;
		Server* _serv;

		void nick(Client* client, const std::vector<std::string>& args);
		void user(Client* client, const std::vector<std::string>& args);
		void pass(Client* client, const std::vector<std::string>& args);
		void privmsg(Client* client, const std::vector<std::string>& args);
		void join(Client* client, const std::vector<std::string>& args);
		
		void authenticated(Client* client);
		std::vector<std::string> parse_join_args(const std::string& str);
		void send_msg_to_client(const int client_fd, const std::string& message) const;
		void reply_to_join(Client* client, Channel* channel) const;
	public:
		Parser(Server* server);
		~Parser();
		void parse(Client* client, const std::string& message);
};
