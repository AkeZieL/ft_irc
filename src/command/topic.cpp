#include "../../include/Parser.hpp"

void Parser::topic(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::string topic;

	//Verifier si le channel existe
	Channel* tmp_channel = this->_serv->get_channel(args[0]);
	if (tmp_channel == NULL) {
		msg_to_client = "403 " + client->get_nickname() + " " +  args[0] + " :No such channel\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//Vérifier si le client est dans le channel
	if (tmp_channel->client_is_in_channel(client) == false) {
		msg_to_client = "441 " + client->get_nickname() + " " + args[0] + " :Client not in channel\r\n";
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//Reconstruire le message si il y en a un
        for(std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); it++) {
                if (!topic.empty()){
                        topic += " ";
                }
                topic += *it;
        }
	//set le topic
	tmp_channel->set_topic(client, topic);
}
