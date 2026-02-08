#include "Server.h"

using namespace ChatServer;

int main() {
    Server server(12345);//tworzy obiekt i naslucuje na porcie 12345
    server.start();//uruchamia serwer
    return 0;
}
