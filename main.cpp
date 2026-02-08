#include "Client.h"
#include "Protocol.h"
#include <iostream>
#include <string>
#include <thread>

using namespace std;

int main() {
    string ip;
    cout << "Podaj IP serwera: ";
    cin >> ip;

    string nick;
    cout << "Podaj nick: ";
    cin >> nick;

    string password;
    cout << "Podaj haslo: ";
    cin >> password;

    ChatClient::Client client(ip, 12345, nick);
    if (!client.connectToServer()) {
        cout << "Nie udalo sie polaczyc z serwerem." << endl;
        system("pause");
        return -1;
    }

    ChatShared::Message login;
    login.type = "login";
    login.from = nick;
    login.text = password;
    client.sendMessage(ChatShared::messageToJson(login) + "\n");

    cout << "Wyslano zadanie logowania..." << endl;

    thread receiver([&]() { client.receiveLoop(); });
    receiver.detach();

    cin.ignore();
    while (true) {
        cout << "> " << flush;
        string line;
        getline(cin, line);

        if (line.empty()) continue;
        if (line == "exit") break;

        ChatShared::Message msg;
        msg.type = "message";
        msg.from = nick;

        if (line[0] == '@') {
            size_t spacePos = line.find(' ');
            if (spacePos != string::npos) {
                msg.to = line.substr(1, spacePos - 1);
                msg.text = line.substr(spacePos + 1);
            }
            else {
                msg.to = line.substr(1);
                msg.text = "";
            }
        }
        else {
            msg.to = "all";
            msg.text = line;
        }

        client.sendMessage(ChatShared::messageToJson(msg) + "\n");
    }

    return 0;
}
