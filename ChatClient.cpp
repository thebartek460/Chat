#include "Client.h"
#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

namespace ChatClient {

    Client::Client(const std::string& h, int p, const std::string& n)
        : host(h), port(p), nick(n) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        sock = socket(AF_INET, SOCK_STREAM, 0);
    }

    Client::~Client() {
        closesocket(sock);
        WSACleanup();
    }

    bool Client::connectToServer() {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) return false;
        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) return false;
        return true;
    }

    void Client::sendMessage(const std::string& msg) {
        std::string msgWithNewline = msg + "\n";  
        send(sock, msgWithNewline.c_str(), (int)msgWithNewline.size(), 0);
    }

    void Client::receiveLoop() {
        char buffer[1024];
        while (true) {
            memset(buffer, 0, 1024);
            int bytesReceived = recv(sock, buffer, 1024, 0);
            if (bytesReceived <= 0) {
                cout << "\n[INFO] Utracono polaczenie z serwerem." << endl;
                break;
            }
            string incomingMsg(buffer, bytesReceived);
            cout << "\r" << incomingMsg << endl;
            cout << "> " << flush;
        }
    }

}
