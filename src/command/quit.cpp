#include "../../include/Parser.hpp"

void Parser::quit(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::string bye_msg;

	if (args.empty()) {
		bye_msg = "Connection closed";
	}
	else {
		for(std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); it++) {
			if (!bye_msg.empty()) {
				bye_msg += " ";
			}
			bye_msg.append(*it);
		}
	}
	msg_to_client = ":" + client->get_nickname() + " QUIT :Quit: " + bye_msg + "\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	this->_serv->client_disconnect(client->get_client_fd());
}
