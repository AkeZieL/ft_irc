#include "Client.hpp"

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
