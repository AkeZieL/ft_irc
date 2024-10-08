#include "../../include/Parser.hpp"

void Parser::names(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
        std::vector<Channel*> channels = this->_serv->get_all_channel();
        std::vector<std::string> channels_to_list;
        Channel* tmp_channel = NULL;
	std::vector<std::vector<Client*> > all_clients; 
	std::string op_nickname;
	std::string reg_nickname;
	std::string all_nickname;


	switch(args.size()) {
		case 0:
			msg_to_client = "461 " + client->get_nickname() + " :NAME command has not enought params\r\n";
			this->send_msg_to_client(client->get_client_fd(), msg_to_client);
			return;
		case 1:
			channels_to_list = parse_join_args(args[0]);
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
        if(channels.empty()) {
                msg_to_client = ":server 403 " + client->get_nickname() + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                return ;
        }
        //Lister les channels + noms par channel
        for(std::vector<Channel*>::const_iterator it = channels.begin(); it != channels.end(); it++) {
                //Si il y a des channels a list en particulier
                if (!channels_to_list.empty()) {
                        //Vérifier dans la liste des channels si on trouve le channel donné
                        for(std::vector<std::string>::const_iterator it2 = channels_to_list.begin(); it2 != channels_to_list.end(); it2++) {
                                if((*it)->get_channel_name() == *it2){
					all_clients = (*it)->get_client_in_channel();
					//channel_client[0] sont les opérateurs. channel_client[1] sont les utilisateurs réguliers (voir get_client_in_channel)
        				for(std::vector<Client*>::iterator it3 = all_clients[0].begin(); it3 != all_clients[0].end(); it3++) {
                				if(!op_nickname.empty()){
                        				op_nickname.append(" ");
                				}
                				op_nickname.append("@");
                				op_nickname.append((*it3)->get_nickname());
        				}
       					for(std::vector<Client*>::iterator it3 = all_clients[1].begin(); it3 != all_clients[1].end(); it3++) {
              					if(!reg_nickname.empty()) {
                					reg_nickname.append(" ");
                				}
                				reg_nickname.append((*it3)->get_nickname());
        				}
        				std::string all_nickname = op_nickname;
        				if (!reg_nickname.empty()) {
						all_nickname += " " + reg_nickname;
                       			}
					//Envoyer message
					msg_to_client = "353 " + client->get_nickname() + " = " + (*it)->get_channel_name() + " :" + all_nickname + "\r\n"; //RPL_NAMEREPLY
					this->send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
			}
                }
                else {
			all_clients = (*it)->get_client_in_channel();
			//channel_client[0] sont les opérateurs. channel_client[1] sont les utilisateurs réguliers (voir get_client_in_channel)
        		for(std::vector<Client*>::iterator it3 = all_clients[0].begin(); it3 != all_clients[0].end(); it3++) {
                		if(!op_nickname.empty()){
                        		op_nickname.append(" ");
                		}
                		op_nickname.append("@");
                		op_nickname.append((*it3)->get_nickname());
        		}
       			for(std::vector<Client*>::iterator it3 = all_clients[1].begin(); it3 != all_clients[1].end(); it3++) {
              			if(!reg_nickname.empty()) {
                			reg_nickname.append(" ");
                		}
                		reg_nickname.append((*it3)->get_nickname());
        		}
        		std::string all_nickname = op_nickname;
        		if (!reg_nickname.empty()) {
				all_nickname += " " + reg_nickname;
                     	}
			msg_to_client = "353 " + client->get_nickname() + " = " + (*it)->get_channel_name() + " :" + all_nickname + "\r\n"; //RPL_NAMEREPLY
			this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                }
		msg_to_client = "366 " + client->get_nickname() + " " + (*it)->get_channel_name() + " :End of name\r\n"; // RPL_ENDOFNAME
        	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
        }
}
