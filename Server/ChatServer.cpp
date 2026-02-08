#include "Server.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

namespace ChatServer {

    std::string extractField(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";

        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";

        size_t startQuote = json.find("\"", pos);
        if (startQuote == std::string::npos) return "";

        size_t endQuote = json.find("\"", startQuote + 1);
        if (endQuote == std::string::npos) return "";

        return json.substr(startQuote + 1, endQuote - startQuote - 1);
    }

    void Server::loadAccountsFromFile() {
        std::ifstream file("accounts.txt");
        if (!file.is_open()) {
            cout << "[INFO] Brak accounts.txt – plik zostanie utworzony." << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            size_t dot = line.find('.');
            if (dot == string::npos) continue;

            string nick = line.substr(0, dot);
            string pass = line.substr(dot + 1);

            accounts[nick] = { nick, pass };
        }

        cout << "[INFO] Zaladowano " << accounts.size() << " kont." << endl;
    }

    bool Server::saveAccountToFile(const std::string& nick, const std::string& password) {
        std::lock_guard<std::mutex> lock(accountsMutex);
        if (accounts.count(nick)) return false;

        ofstream file("accounts.txt", ios::app);
        if (!file.is_open()) return false;

        file << nick << "." << password << endl;
        accounts[nick] = { nick, password };
        return true;
    }

    void adminConsole(Server* server) {
        string cmd;
        while (true) {
            getline(cin, cmd);
            if (cmd.rfind("register.", 0) == 0) {
                string data = cmd.substr(9);
                size_t dot = data.find('.');
                if (dot == string::npos) {
                    cout << "[ERROR] Format: register.nick.haslo" << endl;
                    continue;
                }
                string nick = data.substr(0, dot);
                string pass = data.substr(dot + 1);

                if (server->saveAccountToFile(nick, pass)) {
                    cout << "[OK] Konto utworzone: " << nick << endl;
                }
                else {
                    cout << "[ERROR] Konto juz istnieje." << endl;
                }
            }
        }
    }

    Server::Server(int portNum) : port(portNum), logger("server_chat.log") {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        loadAccountsFromFile();
    }

    Server::~Server() {
        closesocket(listenSocket);
        WSACleanup();
    }

    void Server::start() {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cout << "Bind failed!" << endl;
            return;
        }

        listen(listenSocket, SOMAXCONN);
        thread admin(adminConsole, this);
        admin.detach();
        acceptClients();
    }

    void Server::acceptClients() {
        while (true) {
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                thread(&Server::handleClient, this, clientSocket).detach();
            }
        }
    }

    void Server::handleClient(SOCKET clientSocket) {
        char buffer[8192];
        std::string recvBuffer;   // BUFOR TCP
        bool isLoggedIn = false;
        std::string loggedNick;

        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) break;

            recvBuffer.append(buffer, bytesReceived);

            size_t pos;
            while ((pos = recvBuffer.find('\n')) != std::string::npos) {
                std::string msgStr = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 1);

                ChatShared::Message msg;
                msg.type = extractField(msgStr, "type");
                msg.from = extractField(msgStr, "from");
                msg.to = extractField(msgStr, "to");
                msg.text = extractField(msgStr, "text");

                if (msg.type.empty() || msg.from.empty()) continue;
                // -------- LOGIN --------
                if (msg.type == "login") {
                    std::lock_guard<std::mutex> lockAcc(accountsMutex);

                    if (!accounts.count(msg.from)) {
                        send(clientSocket, "[ERROR] Konto nie istnieje.\n", 26, 0);
                        continue;
                    }
                    if (accounts[msg.from].password != msg.text) {
                        send(clientSocket, "[ERROR] Nieprawidlowe haslo.\n", 30, 0);
                        continue;
                    }

                    {
                        std::lock_guard<std::mutex> lockUsers(usersMutex);
                        if (users.count(msg.from)) {
                            send(clientSocket, "[ERROR] Uzytkownik juz zalogowany.\n", 34, 0);
                            continue;
                        }
                        users[msg.from] = User{ msg.from, clientSocket };
                    }

                    isLoggedIn = true;
                    loggedNick = msg.from;
                    send(clientSocket, "[INFO] Zalogowano pomyslnie.\n", 30, 0);
                    continue;
                }

                // -------- MESSAGE --------
                if (msg.type == "message") {
                    if (!isLoggedIn) {
                        send(clientSocket, "[ERROR] Musisz sie zalogowac.\n", 31, 0);
                        continue;
                    }

                    // LOGOWANIE (ważne w testach!)
                    logger.log(msg.from, msg.to, msg.type, msg.text);

                    std::string formattedMsg;

                    if (msg.to == "all" || msg.to.empty()) {
                        formattedMsg = "[" + msg.from + "]: " + msg.text;

                        std::lock_guard<std::mutex> lock(usersMutex);
                        for (auto& u : users) {
                            if (u.second.socket != clientSocket) {
                                send(u.second.socket,
                                    formattedMsg.c_str(),
                                    (int)formattedMsg.size(),
                                    0);
                            }
                        }
                    }
                    else {
                        std::lock_guard<std::mutex> lock(usersMutex);
                        if (users.count(msg.to)) {
                            formattedMsg = "[PRIV od " + msg.from + "]: " + msg.text;
                            send(users[msg.to].socket,
                                formattedMsg.c_str(),
                                (int)formattedMsg.size(),
                                0);
                        }
                        else {
                            std::string err = "[INFO] Uzytkownik nie jest dostepny.\n";
                            send(clientSocket, err.c_str(), (int)err.size(), 0);
                        }
                    }
                }
            }
        }

        // Sprzątanie po kliencie
        if (isLoggedIn) {
            std::lock_guard<std::mutex> lock(usersMutex);
            users.erase(loggedNick);
        }

        closesocket(clientSocket);
    }
}