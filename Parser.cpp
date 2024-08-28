#include "Parser.hpp"

Parser::Parser(Server* server) : _serv(server) {
	this->_commands["NICK"] = &Parser::nick;
	this->_commands["USER"] = &Parser::user;
	this->_commands["PASS"] = &Parser::pass;
	this->_commands["PRIVMSG"] = &Parser::privmsg;
	this->_commands["JOIN"] = &Parser::join;
}

Parser::~Parser() {
}

void Parser::parse(Client* client, const std::string& message) {
    std::istringstream stream(message);
    std::string command;
    std::vector<std::string> args;
    std::string token;
    std::string msg_to_client;

    try
    {
    	// Lire la commande
        if (!(stream >> command)) {
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n"; // ERR_UNKNOWNCOMMAND
	    this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	    return ;
        }

    	// Lire les arguments
        while (stream >> token) {
            args.push_back(token);
        }

        //Execution de la commande
    	std::map<std::string, commands_ptr>::iterator it = _commands.find(command);
    	if (it != _commands.end()) {
    	    (this->*(it->second))(client, args);
    	}
    	else {
	    msg_to_client =  "421 " + client->get_nickname() + command + " :Unknown command\r\n"; // ERR_UNKNOWNCOMMAND
	    this->send_msg_to_client(client->get_client_fd(), msg_to_client);
    	}
    }
    catch(std::exception &e)
    {
	    std::cerr << "Error : " << e.what() << std::endl;
    }
}

void Parser::nick(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty() || args[0].empty()) {
		msg_to_client = "431 " + client->get_nickname() + " :Nickname not given\r\n"; // ERR_NONICKNAMEGIVEN
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	if(this->_serv->get_client_nickname(args[0]) != NULL) {
		msg_to_client = "433 " + client->get_nickname() + " :Nickname already used\r\n"; // ERR_NICKNAMEINUSE
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	if(client->get_connected() == false){
		client->set_nickname(args[0]);
		this->authenticated(client);
		return ;
	}
	//Si déjà connecté, il faut changer le nickname qui se trouve dans les channels
	std::vector<Channel*> client_channels = client->get_all_channel();
	std::vector<Channel*> server_channels = this->_serv->get_all_channel();
	Client* copy = new Client(*client);
	client->set_nickname(args[0]);

	for(std::vector<Channel*>::const_iterator it_c = client_channels.begin(); it_c != client_channels.end(); it_c++) {
		for(std::vector<Channel*>::const_iterator it_s = server_channels.begin(); it_s != server_channels.end(); it_s++) {
			if((*it_s)->get_channel_name() == (*it_c)->get_channel_name()){
				bool is_operator = this->_serv->get_channel((*it_s)->get_channel_name())->remove_client(copy);
				this->_serv->get_channel((*it_s)->get_channel_name())->set_client(client, is_operator);
				break ;
			}
		}
	}
	client->set_nickname(args[0]);
}

void Parser::user(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.size() < 4) {
		msg_to_client = "461 " + client->get_nickname() + " :USER command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	client->set_username(args[0]);
	client->set_realname(args[3]);
	this->authenticated(client);
}

void Parser::pass(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;

	if (args.empty() || args[0].empty()) {
		msg_to_client = "461 " + client->get_nickname() + " :PASS command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	if (this->_serv->get_password() != args[0]) {
		msg_to_client = "464 " + client->get_nickname() + " :Wrong password\r\n"; // ERR_PASSWDMISMATCH
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	client->set_password(true);
	this->authenticated(client);
}

void Parser::authenticated(Client* client) {
	std::string msg_to_client;

	if (client->get_username().empty() || client->get_realname().empty() || client->get_nickname().empty() || client->get_password() == false) {
		return ;
	}
	client->set_connected(true);
	try
	{
		msg_to_client = "001 " + client->get_nickname() + " :Welcome " + client->get_nickname() + " to my IRC server!\r\n"; // RPL_WELCOME
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
	catch(std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}

void Parser::privmsg(Client* client, const std::vector<std::string>& args) {
	std::string msg_to_client;
	std::string message;

	if(client->get_connected() == false) {
		return ;
	}
	if(args.empty() || args[0].empty() || args[1].empty())
	{
		msg_to_client = "461 " + client->get_nickname() + " :PRIVMSG command has not enought params\r\n"; // ERR_NEEDMOREPARAMS
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//reconstrure le message
	for(std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); it++) {
		if (!message.empty()){
			message += " ";
		}
		message += *it;
	}
	//Envoyer message
	if (args[0][0] == '#'){
		return ;
	}
	else {
		Client* reciever = this->_serv->get_client_nickname(args[0]);
		if (reciever == NULL) {
			msg_to_client = "401 " + client->get_nickname() + " " + args[0] + " :This nickname doesn't exist\r\n"; // ERR_NOSUCHNICK
			send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		}
		msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + args[0] + " " + message + "\r\n";
		this->send_msg_to_client(reciever->get_client_fd(), msg_to_client);
		return ;
	}

}

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
		Channel* tmp_channel = this->_serv->get_channel(channel[i]); //Ce n'est pas une copie, si je modifie tmp_channel, je modifie aussi le channel du serv
		if(tmp_channel == NULL) {
			if(key.empty()) {
				Channel* new_channel = new Channel(channel[i], "", client);
				this->_serv->set_channels(new_channel);
				client->set_channels(new_channel);
				tmp_channel = new_channel;
			}
			else {
				Channel* new_channel = new Channel(channel[i], key[i], client);
				this->_serv->set_channels(new_channel);
				client->set_channels(new_channel);
				tmp_channel = new_channel;
			}
		}
		else {
			//Verifie si le client est déjà dans le channel
			std::vector<std::vector<Client*> > channel_clients = tmp_channel->get_client_in_channel();
			bool is_in_channel = false;
			for(std::vector<Client*>::const_iterator it = channel_clients[0].begin(); it != channel_clients[0].end(); it++){
				if(client->get_nickname() == (*it)->get_nickname()){
					is_in_channel = true;
					break ;
				}
			}
			if(is_in_channel == false) {
				for(std::vector<Client*>::const_iterator it = channel_clients[1].begin(); it != channel_clients[1].end(); it++){
					if(client->get_nickname() == (*it)->get_nickname()){
						is_in_channel = true;
						break ;
					}
				}
			}
			if(is_in_channel == true){
				return ;
			}
			//Check si il y a de la place dans le channel
			if(tmp_channel->get_client_size() >= tmp_channel->get_limit()) {
				msg_to_client = "471 " + client->get_nickname() + " " + channel[i] + " :Channel is full\r\n"; // ERR_CHANNELISFULL
				this->send_msg_to_client(client->get_client_fd(), msg_to_client);
				return ;
			}
			//Check la clé
			if(tmp_channel->get_key() != ""){
				if(key.empty() || key[i] != tmp_channel->get_key()) {
					msg_to_client = "475 " + client->get_nickname() + " " + channel[i] + " :Wrong key\r\n"; // ERR_BADCHANNELKEY
					this->send_msg_to_client(client->get_client_fd(), msg_to_client);
					return ;
				}
			}
			//Ajouter le client au channel
			client->set_channels(tmp_channel);
			tmp_channel->set_client(client, false);
		}
		this->reply_to_join(client, tmp_channel);
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

void Parser::reply_to_join(Client* client, Channel* channel) const {
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
	//Message de retour pour celui qui join	
	msg_to_client = ":" + client->get_nickname() + " JOIN : " + channel->get_channel_name() + "\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);

	msg_to_client = "001 " + client->get_nickname() + " :Welcome " + client->get_nickname() + " to " + channel->get_channel_name() + " channel\r\n";
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	
	std::string topic = channel->get_topic();
	if(topic.empty()){
		msg_to_client = "331 " + client->get_nickname() + " " + channel->get_channel_name() + " :No topic is set\r\n"; // RPL_NOTOPIC
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
	else {
		msg_to_client = "332 " + client->get_nickname() + " " + channel->get_channel_name() + " :" +  topic + "\r\n"; // RPL_TOPIC
		this->send_msg_to_client(client->get_client_fd(), msg_to_client);
	}
	msg_to_client = "353 " + client->get_nickname() + " = " + channel->get_channel_name() + " :" + op_nickname + " " + reg_nickname + "\r\n"; // RPL_NAMEREPLY
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);

	msg_to_client = "366 " + client->get_nickname() + " " + channel->get_channel_name() + " :End of name\r\n"; // RPL_ENDOFNAME
	this->send_msg_to_client(client->get_client_fd(), msg_to_client);

	//Message d'arriver dans le channel pour tout les autres
	for(std::vector<Client*>::const_iterator it = channel_clients[0].begin(); it != channel_clients[0].end(); it++){
		msg_to_client = ":" + client->get_nickname() + " JOIN : " + channel->get_channel_name() + "\r\n";
		this->send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
	for(std::vector<Client*>::const_iterator it = channel_clients[1].begin(); it != channel_clients[1].end(); it++){
		msg_to_client = ":" + client->get_nickname() + " JOIN : " + channel->get_channel_name() + "\r\n";
		this->send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
}

void Parser::send_msg_to_client(const int client_fd, const std::string& message) const{
	if(send(client_fd, message.c_str(), message.size(), 0) < 0)
		throw std::runtime_error("Error while sending message to client");
}
