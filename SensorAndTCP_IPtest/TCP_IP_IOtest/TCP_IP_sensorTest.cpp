#include <stdio.h>          // Librería de entrada/salida
#include <winsock2.h>       // Librería de sockets y red
#include <string.h>         // Librería de cadenas

#pragma comment(lib, "ws2_32.lib") // Enlace con la librería Winsock

#define PORT 12345            // Puerto del servidor ESP32
#define ESP32_IP "192.168.0.192" // Dirección IP fija del ESP32

void connect_to_esp32(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0}; // Buffer para almacenar respuesta
    char message = 'A';      // Mensaje inicial al servidor

    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectado al servidor.\n");

    if (send(*sock, &message, sizeof(message), 0) < 0) {
        printf("Error enviando datos. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Mensaje enviado: %c\n", message);

    int valread = recv(*sock, buffer, sizeof(buffer), 0);
    if (valread > 0) {
        buffer[valread] = 0;
        printf("Respuesta recibida: %s\n", buffer);
    } else {
        printf("Error recibiendo respuesta. Código: %d\n", WSAGetLastError());
    }
}

void view_position(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0};
    char stopCommand[] = "STOP\n";

    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectado al servidor. Mostrando posición en tiempo real...\n");

    while (1) {
        // Limpiar pantalla
        system("cls");

        // Leer datos del servidor
        int valread = recv(*sock, buffer, sizeof(buffer), 0);
        if (valread > 0) {
            buffer[valread] = 0;
            printf("Posición: %s\n", buffer);
        } else {
            printf("Error recibiendo datos. Código: %d\n", WSAGetLastError());
            break;
        }

        // Verificar si el usuario quiere salir del menú
        printf("\nPresiona 's' para salir.\n");
        if (getchar() == 's') {
            // Enviar comando STOP al servidor
            send(*sock, stopCommand, strlen(stopCommand), 0);
            printf("Saliendo del menú...\n");
            break;
        }
    }
}

int main(void) {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    int option;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error inicializando Winsock. Código: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(ESP32_IP);

    while (1) {
        printf("\nOpciones:\n");
        printf("1. Conectar al ESP32 y recibir datos\n");
        printf("2. Ver posición del eslabón en tiempo real\n");
        printf("3. Cerrar el programa\n");
        printf("Elige una opción: ");
        scanf("%d", &option);

        if (option == 1) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            connect_to_esp32(&sock, &server);
            closesocket(sock);

        } else if (option == 2) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            view_position(&sock, &server);
            closesocket(sock);

        } else if (option == 3) {
            closesocket(sock);
            WSACleanup();
            printf("Conexión cerrada. Programa terminado.\n");
            break;
        } else {
            printf("Opción inválida, por favor elige otra.\n");
        }
    }
    return 0;
}
