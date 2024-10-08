#include "../../include/Parser.hpp"

void Parser::list(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::vector<Channel*> channels = this->_serv->get_all_channel();
	std::vector<std::string> channels_to_list;
	Channel* tmp_channel = NULL;

	if(channels.empty()) {
		msg_to_client = ":server 403 " + client->get_nickname() + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
        	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//vérifier si il y a des channels a list en particulier
	if (!args.empty()) {
		channels_to_list = this->parse_join_args(args[0]);
		//Vérifier que les channels a list est valide
		for(std::vector<std::string>::iterator it = channels_to_list.begin(); it != channels_to_list.end();) {
			tmp_channel = this->_serv->get_channel(*it);
			if (tmp_channel == NULL) {
				msg_to_client = ":server 403 " + client->get_nickname() + " " + *it + " :No such channel\r\n";
				this->send_msg_to_client(client->get_client_fd(), msg_to_client);
				it = channels_to_list.erase(it);
				continue ;
			}
			else {
				it++;
			}
		}
	}
	//Lister les channels
	for(std::vector<Channel*>::const_iterator it = channels.begin(); it != channels.end(); it++) {
		//Si il y a des channels a list en particulier
		if (!channels_to_list.empty()) {
			for(std::vector<std::string>::const_iterator it2 = channels_to_list.begin(); it2 != channels_to_list.end(); it2++) {
				//Vérifier dans la liste des channels si on trouve le channel donné
				if((*it)->get_channel_name() == *it2){
					msg_to_client = "322 " + client->get_nickname() + " " + (*it)->get_channel_name() + " " + (*it)->size_t_to_string((*it)->get_client_size()) +  " " + (*it)->get_topic() + "\r\n";
					this->send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
			}
		}
		else {
			msg_to_client = "322 " + client->get_nickname() + " " + (*it)->get_channel_name() + " " + (*it)->size_t_to_string((*it)->get_client_size()) + " " + (*it)->get_topic() + "\r\n";
			std::cout << msg_to_client << std::endl;
			this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		}
	}
	msg_to_client = ":server 323 " + client->get_nickname() + " :End of LIST\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
}
