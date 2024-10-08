#include "../../include/Parser.hpp"

void Parser::kick(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::string comment;
	Channel* tmp_channel;
	Client* tmp_client;

	switch(args.size()) {
		case 0:
			/*FALL_THROUGH*/
		case 1:
			msg_to_client = "461 " + client->get_nickname() + " :KICK Has not enought params\r\n";
			this->send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		case 2:
			tmp_channel = this->_serv->get_channel(args[0]);
			tmp_client = this->_serv->get_client_nickname(args[1]);
			comment = "";
			break ;
		default:
			tmp_channel = this->_serv->get_channel(args[0]);
			tmp_client = this->_serv->get_client_nickname(args[1]);
			for(std::vector<std::string>::const_iterator it = args.begin() + 2; it != args.end(); it++) {
				if (!comment.empty()) {
					comment += " ";
				}
				comment.append(*it);
			}
	}
	if (tmp_channel == NULL) {
		//NOSUCHCHANNEL
	}
	if (tmp_client == NULL) {
		//no client
	}
	if (tmp_channel->client_is_operator(tmp_client) == false) {
		//ERR_CHANOPRIVNEEDED
	}
	tmp_channel->kick(client, tmp_client, comment);
}
