/**
 * @author  Carlos P. M., + Chatgpt o4-mini-high model
 * @date    29 05, 2025
 * @name    C++ Connection to esp32, to read data from the ethernet port
 */

// cout << "\033[2J\033[1;1H";  // will help to clean the cmd

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <limits>
#include <string>
#include <chrono>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class ESP32Client {
    SOCKET sock;
public:
    ESP32Client() : sock(INVALID_SOCKET) {}

    bool connectTo(const string& ipStr, unsigned short port) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) return false;

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        if (InetPtonA(AF_INET, ipStr.c_str(), &addr.sin_addr) != 1) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            return false;
        }
        return true;
    }

    bool requestSensor(int& outValue) {
        if (sock == INVALID_SOCKET) return false;
        char cmd = '1';
        if (send(sock, &cmd, 1, 0) == SOCKET_ERROR) return false;

        char buf[32] = {};
        int ret = recv(sock, buf, sizeof(buf)-1, 0);
        if (ret > 0) {
            buf[ret] = '\0';
            outValue = stoi(buf);
            return true;
        }
        return false;
    }

    void disconnect() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }

    bool isConnected() const {
        return sock != INVALID_SOCKET;
    }
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }

    ESP32Client client;
    int choice = 0;

    while (true) {
        cout << "\n=== ESP32 TCP Client ===\n"
                "1. Connect\n"
                "2. Request sensor data\n"
                "3. Stream data for 10 seconds\n"
                "4. Disconnect\n"
                "5. Exit\n"
                "Select option: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input\n";
            continue;
        }

        if (choice == 1) {
            if (client.isConnected()) {
                cout << "Already connected\n";
            } else if (client.connectTo("192.168.0.100", 5000)) {
                cout << "Connected to 192.168.0.100:5000\n";
            } else {
                cout << "Connection failed\n";
            }

        } else if (choice == 2) {
            if (!client.isConnected()) {
                cout << "Not connected. Use option 1 first.\n";
            } else {
                int value;
                if (client.requestSensor(value)) {
                    cout << "Sensor value: " << value << "\n";
                } else {
                    cout << "Failed to get data\n";
                }
            }

        } else if (choice == 3) {
            if (!client.isConnected()) {
                cout << "Not connected. Use option 1 first.\n";
            } else {
                cout << "Streaming sensor data for 10 seconds...\n";
                auto start = chrono::steady_clock::now();
                while (chrono::duration_cast<chrono::seconds>(
                        chrono::steady_clock::now() - start).count() < 10) {
                    int value;
                    if (client.requestSensor(value)) {
                        cout << value << "\n";
                    } else {
                        cout << "Failed to get data\n";
                    }
                    //this_thread::sleep_for(chrono::milliseconds(200));
                }
                cout << "Done streaming.\n";
            }

        } else if (choice == 4) {
            if (client.isConnected()) {
                client.disconnect();
                cout << "Disconnected\n";
            } else {
                cout << "Not connected\n";
            }

        } else if (choice == 5) {
            if (client.isConnected()) {
                client.disconnect();
                cout << "Disconnected\n";
            }
            cout << "Exiting...\n";
            break;

        } else {
            cout << "Invalid option\n";
        }
    }

    WSACleanup();
    return 0;
}
