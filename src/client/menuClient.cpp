#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <thread>
#include <atomic>


#pragma comment(lib, "Ws2_32.lib")

using namespace std;
using namespace std::chrono;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData)) return 1;

    SOCKET menuSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(6000);
    InetPtonA(AF_INET, "127.0.0.1", &addr.sin_addr);
    cout << "Menu: Connecting to StreamClient...\n";
    if (connect(menuSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Menu: Cannot connect to StreamClient\n";
        WSACleanup(); 
        return 1;
    }
    cout << "Menu: Connected\n";

    cout << "\n1. Connect to ESP32\n"
            << "2. Read one value of the sensor\n"
            << "3. Read sensor until 'q'\n"
            << "4. Disconnect & Exit\n"
            << "Select option: ";    

    int choice;
    while (true) {

        cout <<"Select option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Menu: Invalid\n";
            continue;
        }
        string cmd;
        if (choice == 1) {
            cmd = "CONNECT\n";
            send(menuSock, cmd.c_str(), cmd.size(), 0);
        }
        else if (choice == 2) {
            cmd = "READ1\n";
            send(menuSock, cmd.c_str(), cmd.size(), 0);
        }
        else if (choice == 3) {
            cout << "Reading... press 'q' + Enter to stop.\n";

            std::atomic<bool> stopFlag{false};

            std::thread inputThread([&stopFlag]() {
                char c;
                while (std::cin >> c) {
                    if (c == 'q') {
                        stopFlag = true;
                        break;
                    }
                }
            });

            while (!stopFlag) {
                cmd = "READ1\n";
                send(menuSock, cmd.c_str(), cmd.size(), 0);
                this_thread::sleep_for(milliseconds(1));
            }

            if (inputThread.joinable())
                inputThread.join();
        }

        else if (choice == 4) {
            send(menuSock, "DISCONNECT\n", 11, 0);
            break;
        }
        else {
            cout << "Menu: Invalid\n";
        }
    }

    closesocket(menuSock);
    WSACleanup();
    return 0;
}
