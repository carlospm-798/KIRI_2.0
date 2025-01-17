#include <iostream>
#include <string>
#include <httplib.h>
#include <winsock2.h>
#include <nlohmann/json.hpp>

#pragma comment(lib, "ws2_32.lib") // Conexión a Winsock

#define PORT 12345            // Puerto del ESP32
#define ESP32_IP "192.168.0.192" // IP fija del ESP32

class ESP32Client {
private:
    WSADATA wsa;
    SOCKET sock;
    sockaddr_in server;

public:
    ESP32Client() : sock(INVALID_SOCKET) {
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::runtime_error("Error al inicializar Winsock. Código: " + std::to_string(WSAGetLastError()));
        }
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.s_addr = inet_addr(ESP32_IP);
    }

    ~ESP32Client() {
        closesocket(sock);
        WSACleanup();
    }

    std::string sendCommandToESP32(const std::string& command) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Error al crear socket. Código: " + std::to_string(WSAGetLastError()));
        }

        if (connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
            throw std::runtime_error("Error al conectar con ESP32. Código: " + std::to_string(WSAGetLastError()));
        }

        if (send(sock, command.c_str(), command.length(), 0) < 0) {
            throw std::runtime_error("Error al enviar comando. Código: " + std::to_string(WSAGetLastError()));
        }

        char buffer[1024] = {0};
        int valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
        closesocket(sock);

        if (valread > 0) {
            buffer[valread] = '\0';
            return std::string(buffer);
        } else {
            throw std::runtime_error("Error al recibir respuesta. Código: " + std::to_string(WSAGetLastError()));
        }
    }
};

int main() {
    httplib::Server svr;
    ESP32Client client;

    // Endpoint para procesar comandos
    svr.Post("/command", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = nlohmann::json::parse(req.body);
            std::string command = body["command"];
            std::string response = client.sendCommandToESP32(command);
            res.set_content(response, "text/plain");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error: ") + e.what(), "text/plain");
        }
    });

    // Servir archivos estáticos (HTML, CSS, JS)
    svr.set_mount_point("/", "../public");

    std::cout << "Servidor escuchando en http://localhost:8081\n";
    svr.listen("0.0.0.0", 8081);

    return 0;
}
