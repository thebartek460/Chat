#pragma once
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>
#include <mutex>
#include <winsock2.h>
#include "Logger.h"
#include "Protocol.h"

namespace ChatServer {

    struct User {
        std::string nick;
        SOCKET socket;
    };

    struct Account {
        std::string nick;
        std::string password;
    };

    class Server {
    private:
        std::unordered_map<std::string, User> users;
        std::unordered_map<std::string, Account> accounts;
        std::mutex accountsMutex;
        std::mutex usersMutex;

        ChatShared::Logger logger;
        SOCKET listenSocket;
        int port;

    public:
        Server(int portNum);
        ~Server();
        void start();

        bool saveAccountToFile(const std::string& nick, const std::string& password);
        void loadAccountsFromFile();

    private:
        void acceptClients();
        void handleClient(SOCKET clientSocket);
    };

} // namespace ChatServer
