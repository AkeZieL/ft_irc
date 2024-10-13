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
	//Récupérer tout les channels dont le client fait partie, afin d'envoyer les messages a tout les autres client qui partagent ses channels
	msg_to_client = ":" + client->get_nickname() + " QUIT " + bye_msg + "\r\n";
	std::cout << msg_to_client << std::endl;
	std::vector<Channel*> all_channel = client->get_all_channel();
	for (std::vector<Channel*>::const_iterator it_c = all_channel.begin(); it_c != all_channel.end(); it_c++) {
		std::vector<std::vector<Client*> > all_client_in_channel = (*it_c)->get_client_in_channel();
		for (std::vector<Client*>::const_iterator it = all_client_in_channel[0].begin(); it != all_client_in_channel[0].end(); it++) {
			this->send_msg_to_client((*it)->get_client_fd(), msg_to_client);
		}
		for (std::vector<Client*>::const_iterator it = all_client_in_channel[1].begin(); it != all_client_in_channel[1].end(); it++) {
			this->send_msg_to_client((*it)->get_client_fd(), msg_to_client);
		}
	}
	this->_serv->client_disconnect(client->get_client_fd());
}
