#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Enlazar la biblioteca Winsock

#define PORT 12345

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message = 'A'; // Caracter a enviar
    char buffer[1024] = {0};

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock. Código: %d\n", WSAGetLastError());
        return 1;
    }

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear socket. Código: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurar dirección del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("192.168.0.192"); // Cambia a la IP de tu ESP32

    // Conectar al servidor
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Error al conectar con el servidor. Código: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Conectado al servidor.\n");

    // Enviar datos
    if (send(sock, &message, sizeof(message), 0) < 0) {
        printf("Error al enviar datos. Código: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Mensaje enviado: %c\n", message);

    // Recibir respuesta
    int valread = recv(sock, buffer, sizeof(buffer), 0);
    if (valread > 0) {
        printf("Respuesta recibida: %s\n", buffer);
    } else {
        printf("Error al recibir respuesta. Código: %d\n", WSAGetLastError());
    }

    // Cerrar el socket
    closesocket(sock);
    WSACleanup();

    return 0;
}
