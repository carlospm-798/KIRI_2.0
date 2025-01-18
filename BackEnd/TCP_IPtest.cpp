/**
 * @author Carlos Paredes Márquez
 * @date Jan. 12 2025
 * This code works as an intermediary between 
 * the esp32 as a server, through the nc28j60 
 * ethernet module.
*/


// g++ -o TCP_IPtest.exe TCP_IPtest.cpp -lws2_32

#include <iostream>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Connection to Winsock library
#define PORT 12345            // Server port of the ESP32
#define ESP32_IP "192.168.0.192" // Fixed IP of the ESP32
class ESP32Client {
private:
    WSADATA wsa;
    SOCKET sock;
    sockaddr_in server;
public:
    ESP32Client() : sock(INVALID_SOCKET) {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::runtime_error("Error initializing Winsock. Code: " + std::to_string(WSAGetLastError()));
        }
        // Configure server address
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.s_addr = inet_addr(ESP32_IP);
    }
    ~ESP32Client() {
        closesocket(sock);
        WSACleanup();
    }
    void connectToServer() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Error creating socket. Code: " + std::to_string(WSAGetLastError()));
        }
        if (connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
            throw std::runtime_error("Error connecting to server. Code: " + std::to_string(WSAGetLastError()));
        }
        std::cout << "Connected to server.\n";
    }
    void sendMessage(const std::string& message) {
        if (send(sock, message.c_str(), message.length(), 0) < 0) {
            throw std::runtime_error("Error sending data. Code: " + std::to_string(WSAGetLastError()));
        }
        std::cout << "Message sent: " << message << "\n";
    }
    std::string receiveResponse() {
        char buffer[1024] = {0};
        int valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            std::cout << "Response received: " << buffer << "\n";
            return std::string(buffer);
        } else {
            throw std::runtime_error("Error receiving response. Code: " + std::to_string(WSAGetLastError()));
        }
    }
    void performTask(const std::string& message, bool continuous, int durationMs) {
        try {
            connectToServer();
            sendMessage(message);
            if (continuous) {
                DWORD startTime = GetTickCount();
                while (GetTickCount() - startTime < static_cast<DWORD>(durationMs)) {
                    receiveResponse();
                }
                std::cout << "\nContinuous reading completed.\n";
            } else {
                receiveResponse();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
};
int main() {
    try {
        ESP32Client client;
        int option;
        while (true) {
            std::cout << "\nOptions:\n";
            std::cout << "1. Connect to ESP32 and read a value\n";
            std::cout << "2. Connect to ESP32 and read data continuously for 10 seconds\n";
            std::cout << "3. Reset position to 0\n";
            std::cout << "4. Close the program\n";
            std::cout << ": ";
            std::cin >> option;
            if (option == 1) {
                client.performTask("READ\n", false, 0);
            } else if (option == 2) {
                client.performTask("START\n", true, 10000);
            } else if (option == 3) {
                client.performTask("RESET\n", false, 0);
            } else if (option == 4) {
                std::cout << "Connection closed. Program terminated.\n";
                break;
            } else {
                std::cout << "Invalid option, please choose another one.\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}