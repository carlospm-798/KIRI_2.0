#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

SOCKET menuSock = INVALID_SOCKET;
SOCKET espSock  = INVALID_SOCKET;

bool connectESP() {
    if (espSock != INVALID_SOCKET) return true;

    espSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in eaddr = {};
    eaddr.sin_family = AF_INET;
    eaddr.sin_port   = htons(5000);
    InetPtonA(AF_INET, "192.168.0.100", &eaddr.sin_addr);

    if (connect(espSock, (sockaddr*)&eaddr, sizeof(eaddr)) == SOCKET_ERROR) {
        closesocket(espSock);
        espSock = INVALID_SOCKET;
        return false;
    }
    cout << "Stream: ESP32 connected\n";
    return true;
}

void disconnectESP() {
    if (espSock != INVALID_SOCKET) {
        send(espSock, "0", 1, 0);
        closesocket(espSock);
        espSock = INVALID_SOCKET;
        cout << "Stream: ESP32 disconnected\n";
    }
}

void read() {
    if (!connectESP()) {
        cout << "Stream: Cannot read, ESP32 not connected\n";
        return;
    }
    send(espSock, "1", 1, 0);
    char buf[32];
    int ret = recv(espSock, buf, sizeof(buf)-1, 0);
    if (ret > 0) {
        buf[ret] = '\0';
        cout << buf << "\n";
    } else {
        cout << "Stream: Read failed\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData)) return 1;

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in laddr = {};
    laddr.sin_family = AF_INET;
    laddr.sin_port   = htons(6000);
    InetPtonA(AF_INET, "127.0.0.1", &laddr.sin_addr);
    bind(listenSock, (sockaddr*)&laddr, sizeof(laddr));
    listen(listenSock, 1);
    cout << "Stream: Waiting for MenuClient...\n";
    menuSock = accept(listenSock, NULL, NULL);
    cout << "Stream: MenuClient connected\n";
    closesocket(listenSock);

    ULONG mode = 1;
    ioctlsocket(menuSock, FIONBIO, &mode);

    char recvBuf[32];
    while (true) {
        int ret = recv(menuSock, recvBuf, sizeof(recvBuf)-1, 0);
        if (ret > 0) {
            recvBuf[ret] = '\0';

            if (strncmp(recvBuf, "CONNECT", 7) == 0) connectESP();
            else if (strncmp(recvBuf, "READ1", 5) == 0) read();
            else if (strncmp(recvBuf, "DISCONNECT", 10) == 0) {
                disconnectESP();
                break;
            }
        }


        else if (ret == 0) {
            cout << "Stream: MenuClient disconnected\n";
            break;
        }
        else {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                cout << "Stream: recv error " << err << "\n";
                break;
            }
            this_thread::sleep_for(50ms);
        }
    }

    if (espSock != INVALID_SOCKET) closesocket(espSock);
    if (menuSock != INVALID_SOCKET) closesocket(menuSock);
    WSACleanup();
    return 0;
}
