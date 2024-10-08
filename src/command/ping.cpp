#include "../../include/Parser.hpp"

void Parser::ping(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty()) {
		msg_to_client = "461 " + client->get_nickname() + " :PING has not enought params\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	msg_to_client = ":" + client->get_nickname() + "PONG :localhost\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
}
