#include "../../include/Parser.hpp"

void Parser::join(Client* client, const std::vector<std::string>& args) {
        std::string msg_to_client;
        std::vector<std::string> key;
        std::vector<std::string> channel;

        //deuxieme parsing pour extraire channel et mdp
        switch(args.size()) {
                case 0:
                        msg_to_client = "461 " + client->get_nickname() + " :JOIN command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                case 1:
                        channel = parse_join_args(args[0]);
                        break ;
                case 2:
                        channel = parse_join_args(args[0]);
                        key = parse_join_args(args[1]);
                        if (channel.size() != key.size()){
                                msg_to_client = "461 " + client->get_nickname() + " :JOIN command needs as many channels as key -> JOIN #channel1,#channel2 passwd1,passwd2\r\n"; // ERR_NEEDMOREPARAMS
                                this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                                return ;
                        }
                        break ;
        };
        //crée ou rejoindre le channel
	this->create_or_join_channel(client, channel, key);
}

void Parser::create_or_join_channel(Client* client, std::vector<std::string> channel, std::vector<std::string> key) {
	std::string msg_to_client;
	Channel* tmp_channel = NULL;

        for(size_t i = 0; i != channel.size(); i++) {
                //Check si le nom du channel est valide
                if(channel[i][0] != '#'){
                        msg_to_client = "403 " + client->get_nickname() + " " + channel[i] + " :No such channel\r\n";
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        continue ;
                }
                //trop de channel pour le client
                if(client->get_nbr_channel() > MAX_CHANNEL) {
                        msg_to_client = "405 " + client->get_nickname() + " " + channel[i] + " :You have join too many channels\r\n"; // ERR_TOOMANYCHANNEL
                        this->send_msg_to_client(client->get_client_fd(), msg_to_client);
                        return ;
                }
                //Check si le channel existe ou pas
                tmp_channel = this->_serv->get_channel(channel[i]); //Ce n'est pas une copie, si je modifie tmp_channel, je modifie aussi le channel du serv
                if(tmp_channel == NULL) {
			tmp_channel = this->_serv->create_channel(client, channel, key, i);
			Parser::reply_to_join(client, tmp_channel);
                }
                else {
			if(key.empty())
				client->join_channel(client, "", tmp_channel);
			else
				client->join_channel(client, key[i], tmp_channel);
                }
        }
}

std::vector<std::string> Parser::parse_join_args(const std::string& str) {
        std::vector<std::string> container;
        std::stringstream ss(str);
        std::string arg;

        while(std::getline(ss, arg, ',')) {
                if(!arg.empty()) {
                        container.push_back(arg);
                }
        }
        return (container);
}

void Parser::reply_to_join(Client* client, Channel* channel) {
        std::string msg_to_client;
        std::string op_nickname;
        std::string reg_nickname;
        std::vector<std::vector<Client*> > channel_clients = channel->get_client_in_channel();

        //channel_client[0] sont les opérateurs. channel_client[1] sont les utilisateurs réguliers (voir get_client_in_channel)
        for(std::vector<Client*>::iterator it = channel_clients[0].begin(); it != channel_clients[0].end(); it++) {
                if(!op_nickname.empty()){
                        op_nickname.append(" ");
                }
                op_nickname.append("@");
                op_nickname.append((*it)->get_nickname());
        }
        for(std::vector<Client*>::iterator it2 = channel_clients[1].begin(); it2 != channel_clients[1].end(); it2++) {
                if(!reg_nickname.empty()) {
                        reg_nickname.append(" ");
                }
                reg_nickname.append((*it2)->get_nickname());
        }
        std::string all_nickname = op_nickname;
        if (!reg_nickname.empty())
                all_nickname += " " + reg_nickname;
        //Message de retour pour celui qui join
        //msg de bienvenue
        msg_to_client = ":" + client->get_nickname() + " JOIN :" + channel->get_channel_name() + "\r\n";
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);

        msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + channel->get_channel_name() + " :Welcome to " + channel->get_channel_name() + " channel\r\n";
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
        //topic
        std::string topic = channel->get_topic();
        if(topic.empty()){
                msg_to_client = "331 " + client->get_nickname() + " " + channel->get_channel_name() + " :No topic is set\r\n"; // RPL_NOTOPIC
		Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
        }
        else {
                msg_to_client = "332 " + client->get_nickname() + " " + channel->get_channel_name() + " " +  topic + "\r\n"; // RPL_TOPIC
		Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
        }
        //reply noms
        msg_to_client = "353 " + client->get_nickname() + " = " + channel->get_channel_name() + " :" + all_nickname + "\r\n"; // RPL_NAMEREPLY
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
        msg_to_client = "366 " + client->get_nickname() + " " + channel->get_channel_name() + " :End of name\r\n"; // RPL_ENDOFNAME
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
        //afficher liste de noms
        msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + channel->get_channel_name() + " :" + all_nickname + "\r\n";
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);

        //Message d'arriver dans le channel pour tout les autres
        for(std::vector<Client*>::const_iterator it = channel_clients[0].begin(); it != channel_clients[0].end(); it++){
                msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + channel->get_channel_name() + " :Welcome " + client->get_nickname() + " to " + channel->get_channel_name() + " channel" + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
        }
        for(std::vector<Client*>::const_iterator it = channel_clients[1].begin(); it != channel_clients[1].end(); it++){
                msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + channel->get_channel_name() + " :Welcome " + client->get_nickname() + " to " + channel->get_channel_name() + " channel" + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
        }
}
