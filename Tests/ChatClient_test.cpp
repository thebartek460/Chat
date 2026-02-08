#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include "Client.h"
#include "Protocol.h"

using namespace std;

void userRoutine(int id) {
    string nick = "test" + to_string(id);
    string password = "bogdan";

    ChatClient::Client c("127.0.0.1", 12345, nick);

    if (!c.connectToServer()) {
        cout << "Nie mozna polaczyc: " << nick << endl;
        return;
    }

    ChatShared::Message loginMsg;
    loginMsg.type = "login";
    loginMsg.from = nick;
    loginMsg.to = "";
    loginMsg.text = password;

    c.sendMessage(ChatShared::messageToJson(loginMsg) + "\n");

    this_thread::sleep_for(chrono::milliseconds(100));

    for (int i = 1; i <= 10; ++i) {
        ChatShared::Message msg;
        msg.type = "message";
        msg.from = nick;
        msg.to = "all";
        msg.text = "SPAM " + to_string(i);

        c.sendMessage(ChatShared::messageToJson(msg) + "\n");
        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main(int argc, char* argv[]) {

    // jeżeli program został uruchomiony z argumentem id użytkownika to jest to nowy proces
    if (argc == 2) {
        int id = atoi(argv[1]);
        userRoutine(id);
        return 0;
    }

    const int USERS = 100;

    for (int i = 1; i <= USERS; ++i) {
        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        char cmd[256];
        sprintf_s(cmd, "\"%s\" %d", argv[0], i);

        if (!CreateProcessA( ///https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
            NULL,           // lpApplicationName NULL = Windows użyje exe wskazanego w lpCommandLine
            cmd,            // lpCommandLine np. "chat_client_test.exe 7" exe + argument = id użytkownika
            NULL,           // lpProcessAttributes domyślne ustawienia bezpieczeństwa
            NULL,           // lpThreadAttributes NULL = domyślne ustawienia bezpieczeństwa głównego wątku
            FALSE,          // bInheritHandles FALSE = proces nie dziedziczy każdy klient jest niezależny
            CREATE_NEW_CONSOLE, // Tworzy nową konsolę dla każdego uruchomionego procesu
            NULL,           // lpEnvironment
            NULL,           // lpCurrentDirectory
            &si,            // lpStartupInfo określa ustawienia okna konsoli, stdin, stdout itd.
            &pi             // lpProcessInformation windows wypełnia uchwyty procesu i głównego wątku, oraz ID procesu
        )) {
            cout << "Nie mozna uruchomic procesu " << i
                << " err=" << GetLastError() << endl;
        }
        else {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        Sleep(50);
    }

    cout << "TEST ZAKONCZONY" << endl;
    return 0;
}
