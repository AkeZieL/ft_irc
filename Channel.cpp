#include "Channel.hpp"

Channel::Channel(const std::string& channel_name, const std::string& key, Client* creator) : _channel_name(channel_name), _creator(creator), _invite(false), _topic(false), _key(key), _limit(10) {
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
	return "";
}

//setter
void Channel::set_client(Client* client, bool is_operator) {
	if(is_operator == true){
		this->_operator.push_back(client);
		return ;
	}
	this->_regular_user.push_back(client);
}
