#pragma once
#include <string>

namespace ChatClient {

    class Client {
    public:
        Client(const std::string& host, int port, const std::string& nick);
        ~Client();

        bool connectToServer();
        void sendMessage(const std::string& msg);
        void receiveLoop();

    private:
        std::string host;
        int port;
        std::string nick;
        int sock;
    };

}
