#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData)) return 1;

    SOCKET espSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in eaddr = {};
    eaddr.sin_family = AF_INET;
    eaddr.sin_port   = htons(5000);
    InetPtonA(AF_INET, "192.168.0.100", &eaddr.sin_addr);

    cout << "Connecting to ESP32...\n";
    if (connect(espSock, (sockaddr*)&eaddr, sizeof(eaddr)) == SOCKET_ERROR) {
        cerr << "Cannot connect to ESP32\n";
        closesocket(espSock);
        WSACleanup();
        return 1;
    }
    cout << "Connected!\n";

    while (true) {
        cout << "angle:speed: ";
        string input;
        cin >> input;

        if (input == "q") {
            send(espSock, "0", 1, 0);
            break;
        }

        auto sepPos = input.find(':');
        if (sepPos == string::npos) {
            cout << "Invalid format! Use angle:speed\n";
            continue;
        }

        string angleStr = input.substr(0, sepPos);
        string speedStr = input.substr(sepPos + 1);

        try {
            float angle = stof(angleStr);
            float speed = stof(speedStr);

            if (angle < 0.0f || angle > 360.0f || speed < 0.0f || speed > 100.0f) {
                cout << "Invalid values\n";
                continue;
            }

            string cmdStr = "TARGET:" + angleStr + ":" + speedStr + "\n";
            send(espSock, cmdStr.c_str(), cmdStr.size(), 0);
        } catch (...) {
            cout << "Invalid number!\n";
        }
    }

    closesocket(espSock);
    WSACleanup();
    return 0;
}
