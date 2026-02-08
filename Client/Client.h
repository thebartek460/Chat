#pragma once
#include <string>
#include <winsock2.h>

namespace ChatClient {

    class Client {
    private:
        std::string host;
        int port;
        std::string nick;
        SOCKET sock;

    public:
        Client(const std::string& h, int p, const std::string& n);
        ~Client();

        bool connectToServer();
        void sendMessage(const std::string& msg);
        void receiveLoop();
    };

}
