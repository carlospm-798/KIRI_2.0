#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include <thread>
#include <algorithm>

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

    const char* menu = 
        "\n1. Connect to ESP32\n"
        "2. Read one value of the sensor\n"
        "3. Read sensor for 10 seconds\n"
        "4. Move motor to angle\n"
        "5. Disconnect & Exit\n"
        "Select option: ";  

    int choice;
    cout << menu;

    while (true) {

        cout <<"Select option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Menu: Invalid\n";
            continue;
        }
        const char* cmd = nullptr;

        if (choice == 1) cmd = "CONNECT\n";
        else if (choice == 2) cmd = "READ1\n";
        else if (choice == 3) {
            auto start = steady_clock::now();
            while (duration_cast<seconds>(steady_clock::now() - start).count() < 10) {
                send(menuSock, "READ1\n", 6, 0);
                this_thread::sleep_for(1ms);
            }
            continue;
        }

        else if (choice == 4){
            string input;
            cin >> input;

            input.erase(remove_if(input.begin(), input.end(),
                [](char c){ return !isdigit(c) && c != '.';}),
                input.end());
            
            try {
                float angle = stof(input);
                if (angle < 0.0f || angle > 360.0f) {
                    cout << "Angle out of range!\n";
                    continue;
                }
                string cmdStr = "GOTO:" + input + "\n";
                send(menuSock, cmdStr.c_str(), cmdStr.size(), 0);
            } catch (error_code) {
                cout << "Invalid number!\n";
            }
        }

        else if (choice == 5) {
            cmd = "DISCONNECT\n";
            send(menuSock, cmd, 11, 0);
            break;
        }
        else {
            cout << "Menu: Invalid\n";
            continue;
        }

        if (cmd) send(menuSock, cmd, strlen(cmd), 0);
    }

    closesocket(menuSock);
    WSACleanup();
    return 0;
}
