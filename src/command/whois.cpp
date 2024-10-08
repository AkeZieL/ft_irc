#include "../../include/Parser.hpp"

void Parser::whois(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	Client* tmp_client = NULL;

	if (args.empty()) {
		msg_to_client = "431 " + client->get_nickname() + " :No nickname given\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	tmp_client = this->_serv->get_client_nickname(args[0]);
	if (tmp_client == NULL) {
		//ERR_NOSUCHCLIENT
		return ;
	}
	msg_to_client = "311 " + client->get_nickname() + " " + tmp_client->get_nickname() + " " + tmp_client->get_username() + " :" + tmp_client->get_realname() + "\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);

	msg_to_client = "318 " + client->get_nickname() + " " + tmp_client->get_nickname() + " :End of WHOIS list\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
}
