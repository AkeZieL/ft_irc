#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class Server;
class Client;
class Channel;
class Parser;

typedef void (Parser::*commands_ptr)(Client*, const std::vector<std::string>&);

class Parser {
	private:
		Parser();

		std::map<std::string, commands_ptr> _commands;
		Server* _serv;

		//command
		void nick(Client* client, const std::vector<std::string>& args);
		void user(Client* client, const std::vector<std::string>& args);
		void pass(Client* client, const std::vector<std::string>& args);
		void privmsg(Client* client, const std::vector<std::string>& args);
		void join(Client* client, const std::vector<std::string>& args);
		void part(Client* client, const std::vector<std::string>& args);
		//
		void mode(Client* client, const std::vector<std::string>& args);
		void topic(Client* client, const std::vector<std::string>& args);
		void list(Client* client, const std::vector<std::string>& args);
		void names(Client* client, const std::vector<std::string>& args);
		//A VERIFIER EN PRIORITE
		void whois(Client* client, const std::vector<std::string>& args);
		void quit(Client* client, const std::vector<std::string>& args);
		void kick(Client* client, const std::vector<std::string>& args);
		void ping(Client* client, const std::vector<std::string>& args);
		void pong(Client* client, const std::vector<std::string>& args);
		
		void authenticated(Client* client);
		//utils
		std::vector<std::string> parse_join_args(const std::string& str);
		void create_or_join_channel(Client* client, std::vector<std::string> channel, std::vector<std::string> key);
		void part_channel(Client* client, std::vector<std::string> channel, std::string message);
		void send_privmsg(Client* client, std::string message, const std::vector<std::string>& args);
		void mode_check_option(Client* client, std::string command, std::vector<std::string> command_arg, Channel* tmp_channel);

	public:
		Parser(Server* server);
		~Parser();
		void parse(Client* client, const std::string& message);

		//réponse
		static void send_msg_to_client(const int client_fd, const std::string& message);
		static void reply_to_join(Client* client, Channel* channel);
};
