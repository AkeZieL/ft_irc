#include "../include/Client.hpp"
#include "../include/Parser.hpp"

Client::Client(int client_fd) : _client_fd(client_fd), _password(false), _connected(false){
}

Client &Client::operator=(const Client& copy) {
	if (this != &copy) {
		_nickname = copy._nickname;
	}
	return *this;
}

Client::Client(const Client &copy) : _nickname(copy._nickname) {
}

Client::~Client() {
}

void Client::join_channel(Client* client, std::string key, Channel* channel) {
	std::string msg_to_client;

	//Verifie si le client est déjà dans le channel
	if(channel->client_is_in_channel(client) == true){
		return ;
	}
	//Check si il y a de la place dans le channel
	if(channel->get_client_size() >= channel->get_limit()) {
		msg_to_client = "471 " + client->get_nickname() + " " + channel->get_channel_name() + " :Channel is full\r\n"; // ERR_CHANNELISFULL
		Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
		return ;
	}
	//Check la clé
	if(channel->get_key() != ""){
		if(key.empty() || key != channel->get_key()) {
			msg_to_client = "475 " + client->get_nickname() + " " + channel->get_channel_name() + " :Wrong key\r\n"; // ERR_BADCHANNELKEY
			Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
			return ;
		}
	}
	//Ajouter le client au channel
	client->set_channels(channel);
	channel->set_client(client, false);
	Parser::reply_to_join(client, channel);
}

void Client::part_channel(Client* client, Channel* channel, std::string message) {
	std::string msg_to_client;
	std::vector<std::vector<Client*> > channel_clients = channel->get_client_in_channel();

	//Quitter le channel et envoyer le message si il y en a un
	channel->remove_client(client);
	msg_to_client = ":" + client->get_nickname() + " PART :" + channel->get_channel_name() + "\r\n";
	Parser::send_msg_to_client(client->get_client_fd(), msg_to_client);
	if (message != "") {
		//envoyer le message si il y en a un
		std::string tmp = ":WeeChat 4.5.0-dev";
		if(message != tmp){
			msg_to_client = ":" + client->get_nickname() + " PRIVMSG " + channel->get_channel_name() + " " + message + "\r\n";
			for(std::vector<Client*>::const_iterator it = channel_clients[0].begin(); it != channel_clients[0].end(); it++){
				Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
			}
			for(std::vector<Client*>::const_iterator it = channel_clients[1].begin(); it != channel_clients[1].end(); it++){
				Parser::send_msg_to_client((*it)->get_client_fd(), msg_to_client);
			}
		}
	}	
}

void Client::remove_channel(Channel* channel) {
	for(std::vector<Channel*>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++) {
		if((*it)->get_channel_name() == channel->get_channel_name()) {
			this->_channels.erase(it);
			std::cout << "Channel size : " << this->_channels.size() << std::endl;
			return ;
		}
	}
}

//getter
int Client::get_client_fd() const {
    return (this->_client_fd);
}

std::string Client::get_nickname() const {
	return(this->_nickname);
}

std::string Client::get_username() const {
	return(this->_username);
}

std::string Client::get_realname() const {
	return(this->_realname);
}

bool Client::get_password() const {
	return(this->_password);
}

bool Client::get_connected() const {
	return(this->_connected);
}

size_t Client::get_nbr_channel() const {
	return(this->_channels.size());
}

std::vector<Channel*> Client::get_all_channel() const {
	return(this->_channels);
}

//setter

void Client::set_nickname(const std::string& nickname){
	this->_nickname = nickname;
}

void Client::set_realname(const std::string& realname){
	this->_realname = realname;
}

void Client::set_username(const std::string& username){
	this->_username = username;
}

void Client::set_password(const bool& password) {
	this->_password = password;
}

void Client::set_connected(const bool& connected) {
	this->_connected = connected;
}

void Client::set_channels(Channel* channel) {
	this->_channels.push_back(channel);
}
