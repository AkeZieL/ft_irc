#pragma once

#include <string>

class Client {
    private:
        Client();
        
	int _client_fd;
	bool _password;
	bool _connected;
	std::string _nickname;
	std::string _username;
	std::string _realname;
   
    public:
        Client(int client_fd);
        ~Client();

	//getter
        int get_client_fd() const;
	std::string get_nickname() const;
	std::string get_username() const;
	std::string get_realname() const;
	bool get_password() const;
	bool get_connected() const;

	//setter
	void set_nickname(const std::string& nickname);
	void set_realname(const std::string& realname);
	void set_username(const std::string& username);
	void set_password(const bool& password);
	void set_connected(const bool& connected);
};
