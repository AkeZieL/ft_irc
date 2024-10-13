#include "../include/Channel.hpp"

Channel::Channel(const std::string& channel_name, const std::string& key, Client* creator, Server* server) : _serv(server), _channel_name(channel_name), _creator(creator), _topic(""), _invite(false), _bool_topic(false), _key(key), _limit(10) {
	this->_operator.push_back(creator);
}

Channel::~Channel() {
}

bool Channel::remove_client(Client* client) {
	for(std::vector<Client*>::iterator it = this->_operator.begin(); it != this->_operator.end(); it++) {
		if((*it)->get_nickname() == client->get_nickname()) {
			this->_operator.erase(it);
			return true;
		}
	}
	for(std::vector<Client*>::iterator it2 = this->_regular_user.begin(); it2 != this->_regular_user.end(); it2++) {
		if((*it2)->get_nickname() == client->get_nickname()) {
			this->_regular_user.erase(it2);
			break ;
		}
	}
	return false;
}

bool Channel::client_is_in_channel(Client* client) const {
	std::vector<std::vector<Client*> > channel_clients = this->get_client_in_channel();
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
	return is_in_channel;
}

void Channel::add_mode(Client* client, std::string command, std::vector<std::string> command_arg) {
	std::string msg_to_client;
	size_t count_arg = 0;
	char option;
	std::vector<std::string>::iterator it = command_arg.begin();
	Client* tmp_client = NULL;

	//k l o necessite des argument si il y a un +
	//Vérifier si il y autant d'arguement que d'option qui en a besoin
	for (size_t i = 1; i < command.size(); i++) {
		if (command[i] == 'k' || command[i] == 'l' || command[i] == 'o')
			count_arg++;
	}
	if (count_arg != command_arg.size()) {
		msg_to_client = "461 " + client->get_nickname() + " :MODE need as much option as argument\r\n"; // ERR_NEEDMOREPARAM
		Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//Ajouter les options
	for (size_t i = 1; i != command.size(); i++) {
		option = command[i];
		switch (option) {
			case 'k':
				this->_key = *it;
				it++;
				break ;
			case 'l':
				try {
					this->_limit = string_to_size_t(*it);
					it++;
				}
				catch (std::exception &e) {
					msg_to_client = ":" + client->get_nickname() + this->get_channel_name() + " :" + e.what() + "\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
				break ;
			case 'o':
				tmp_client = this->_serv->get_client_nickname(*it);
				//le client à ajouter en opérateur n'existe pas
				if (tmp_client == NULL) {
					msg_to_client = "401 " + *it + " :No such nick\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
					break ;
				}
				//le client à ajouter en opérateur n'est pas dans le channel
				if (client_is_in_channel(tmp_client) == false) {
					msg_to_client = "441 " + client->get_nickname() + " " + *it + " " + " :" + *it + "Isn't on that channel\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
			      		break ;
				}
				this->remove_client(tmp_client);
				this->set_client(tmp_client, true);
				msg_to_client = "001 @" + tmp_client->get_nickname() + " :\r\n";
				Parser::send_msg_to_client(tmp_client->get_client_fd(), msg_to_client);
				it++;
				break ;
			case 't':
				this->_bool_topic = true;
				break ;
			case 'i':
				this->_invite = true;
				break ;
		}
	}
}

void Channel::remove_mode(Client* client, std::string command, std::vector<std::string> command_arg) {
	std::string msg_to_client;
	size_t count_arg = 0;
	char option;
	std::vector<std::string>::iterator it = command_arg.begin();
	Client* tmp_client;

	//seulement o necessite un argument si il y a un -
	//Vérifier si il y autant d'arguement que d'option qui en a besoin
	for (size_t i = 1; i < command.size(); i++) {
		if (command[i] == 'o')
			count_arg++;
	}
	if (count_arg != command_arg.size()) {
		msg_to_client = "461 " + client->get_nickname() + " :MODE need as much option as argument\r\n"; // ERR_NEEDMOREPARAM
		Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//Enlever les options
	for (size_t i = 1; i != command.size(); i++) {
		option = command[i];
		switch (option) {
			case 'k':
				this->_key = "";
				break ;
			case 'l':
				this->_limit = 65535; // valeur max pour un size_t
				break ;
			case 'o':
				tmp_client = this->_serv->get_client_nickname(*it);
				//le client à ajouter en opérateur n'existe pas
				if (tmp_client == NULL) {
					msg_to_client = "401 " + *it + " :No such nick\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
					break ;
				}
				//le client à ajouter en opérateur n'est pas dans le channel
				if (client_is_in_channel(tmp_client) == false) {
					msg_to_client = "441 " + client->get_nickname() + " " + *it + " " + " :" + *it + "Isn't on that channel\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
					break ;
				}
				this->remove_client(tmp_client);
				this->set_client(tmp_client, false);
				msg_to_client = "001 " + tmp_client->get_nickname() + " :\r\n";
				Parser::send_msg_to_client(tmp_client->get_client_fd(), msg_to_client);
				it++;
				break ;
			case 't':
				this->_bool_topic = false;
				break ;
			case 'i':
				this->_invite = false;
				break ;
		}
	}
}

void Channel::mode_states(Client* client, std::string command) const {
	char option;
	std::string msg_to_client;

	for (size_t i = 0; i != command.size(); i++) {
		option = command[i];
		switch (option) {
			case 'k':
				msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :" + this->get_key() + "\r\n";
				Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				break ;
			case 'l':
				msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :" + size_t_to_string(this->get_limit()) + "\r\n";
				Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				break ;
			case 'o':
				for (std::vector<Client*>::const_iterator it = this->_operator.begin(); it != _operator.end(); it++) {
					msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :" + (*it)->get_nickname() + "\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
				for (std::vector<Client*>::const_iterator it = this->_regular_user.begin(); it != _regular_user.end(); it++) {
					msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :" + (*it)->get_nickname() + "\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
				break ;
			case 't':
				msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :" + this->get_topic() + "\r\n";
				Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				break ;
			case 'i':
				if (this->_invite == false) {
					msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :invite only off\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
				else {
					msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " :invite only on\r\n";
					Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
				}
				break ;
		}
	}
}

void Channel::kick(Client* client, Client* client_to_kick, std::string comment) {
	std::string msg_to_client;

	msg_to_client = ":" + client->get_nickname() + " KICK " + this->get_channel_name() + " " + client_to_kick->get_nickname() + " " + comment + "\r\n";
	for(std::vector<Client*>::const_iterator it = this->_operator.begin(); it != this->_operator.end(); it++) {
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
	for(std::vector<Client*>::const_iterator it = this->_regular_user.begin(); it != this->_regular_user.end(); it++) {
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
	this->remove_client(client_to_kick);
	client_to_kick->remove_channel(this);
}

void Channel::send_msg_to_channel(Client* client, std::string message) {
	std::string msg_to_client;

	for(std::vector<Client*>::const_iterator it = this->_operator.begin(); it != this->_operator.end(); it++) {
		if ((*it)->get_nickname() == client->get_nickname()) {
			continue ;
		}
		msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " " + message + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
	for(std::vector<Client*>::const_iterator it = this->_regular_user.begin(); it != this->_regular_user.end(); it++) {
		if ((*it)->get_nickname() == client->get_nickname()) {
			continue ;
		}
		msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + this->get_channel_name() + " " + message + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
}

size_t Channel::string_to_size_t(const std::string& str) const{
	char* end;
	size_t max_size_t = std::numeric_limits<size_t>::max();
	long result = std::strtol(str.c_str(), &end, 10);

	//Si end pointe au début de la chaine, il n'y a pas de chiffre dans str
	if (end == str.c_str()) {
		throw std::runtime_error("MODE +l requieres a numeric value");
	}
	if (*end != '\0') {
		throw std::runtime_error("MODE +l requieres a numeric value");
	}
	//Nbr négatif ou trop grand
	if (result < 0 || static_cast<size_t>(result) > max_size_t) {
		throw std::runtime_error("Number out of range [0 , size_t max]");
	}
	return (static_cast<size_t>(result));
}

std::string Channel::size_t_to_string(const size_t& limit) const {
	std::ostringstream oss;
	oss << limit;

	return(oss.str());
}
//getter
const std::string Channel::get_channel_name() const {
	return(this->_channel_name);
}

size_t Channel::get_limit() const {
	return(this->_limit);
}

size_t Channel::get_client_size() const {
	return(this->_operator.size() + this->_regular_user.size());
}

std::string Channel::get_key() const {
	return(this->_key);
}

std::vector<std::vector<Client*> > Channel::get_client_in_channel() const {
	std::vector<std::vector<Client*> > all_client;

	all_client.push_back(this->_operator);
	all_client.push_back(this->_regular_user);
	return(all_client);
}

std::string Channel::get_topic() const {
	return this->_topic;
}

bool Channel::client_is_operator(Client* client) const {
	for(std::vector<Client*>::const_iterator it = this->_operator.begin(); it != this->_operator.end(); it++) {
		if (client->get_nickname() == (*it)->get_nickname())
			return true;
	}
	return false;
}

//setter

void Channel::set_nickname(Client* client, std::string new_nickname) {
	for (std::vector<Client*>::iterator it = this->_operator.begin(); it != this->_operator.end(); it++) {
		if ((*it)->get_nickname() == client->get_nickname()) {
			(*it)->set_nickname(new_nickname);
			return ;
		}
	}
	for (std::vector<Client*>::iterator it = this->_regular_user.begin(); it != this->_regular_user.end(); it++) {
		if ((*it)->get_nickname() == client->get_nickname()) {
			(*it)->set_nickname(new_nickname);
			return ;
		}
	}
}

void Channel::set_client(Client* client, bool is_operator) {
	if(is_operator == true){
		this->_operator.push_back(client);
		return ;
	}
	this->_regular_user.push_back(client);
}

void Channel::set_topic(Client* client, std::string topic) {
	std::string msg_to_client;
	std::string topic_to_send;
	bool is_new_topic = false;

	if (this->_bool_topic == true) {
		if (this->client_is_operator(client) == false) {
			msg_to_client = "482 " + client->get_nickname() + " " + this->get_channel_name() + " :You are not operator (+t mode active)\r\n";
			Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		}
	}
	//Envoyer le nouveau topic a tout les utilisateur
	switch (topic.size()) {
		case 0:
			// Il faut afficher le topic
			topic_to_send = this->_topic;
			break ;
		case 1:
			// Supprimer le topic actuel
			if (topic[0] == ':') {
				topic_to_send = "There is no topic";
				this->_topic = "";
			}
			break ;
		default:
			// Changer le topic (vérifier si topic commence bien par un : dans le cas contraire seul le premier mot seras pris en compte)
			if (topic[0] != ':') {
				size_t pos = topic.find(' ');

				topic_to_send = topic.substr(0, pos);
				this->_topic = topic_to_send;
				is_new_topic = true;
				break ;
			}
			topic_to_send = topic.substr(1);
			this->_topic = topic_to_send;
			is_new_topic = true;
			break ;
	}
	for (std::vector<Client*>::const_iterator it = _operator.begin(); it != _operator.end(); it++) {
		if (is_new_topic == true) {
			msg_to_client = ":server PRIVMSG " + this->get_channel_name() + " :NEW TOPIC\r\n";
			Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
		}
		msg_to_client = ":server PRIVMSG " + this->get_channel_name() + " :" + topic_to_send + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
	for (std::vector<Client*>::const_iterator it = this->_regular_user.begin(); it != this->_regular_user.end(); it++) {
		if (is_new_topic == true) {
			msg_to_client = ":server PRIVMSG " + this->get_channel_name() + " :NEW TOPIC\r\n";
			Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
		}
		msg_to_client = ":server PRIVMSG " + this->get_channel_name() + " :" + topic_to_send + "\r\n";
		Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
	}
}
