class Client {
    private:
        Client();
        int _client_fd;
    public:
        Client(int client_fd);
        ~Client();

        int get_client_fd() const;
};