#include <stdio.h>          // Librería de entrada/salida
#include <winsock2.h>       // Librería de sockets y red
#include <string.h>         // Librería de cadenas

#pragma comment(lib, "ws2_32.lib") // Enlace con la librería Winsock

#define PORT 12345            // Puerto del servidor ESP32
#define ESP32_IP "192.168.0.192" // Dirección IP fija del ESP32

void connect_to_esp32(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0}; // Buffer para almacenar respuesta
    char message[] = "READ\n"; // Mensaje "READ" que se enviará al ESP32

    // Conectar al servidor
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectado al servidor.\n");

    // Enviar el comando "READ" al ESP32
    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error enviando datos. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Mensaje enviado: %s", message);

    // Leer la respuesta del ESP32
    int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
    if (valread > 0) {
        buffer[valread] = '\0'; // Asegurar que el buffer sea una cadena válida
        printf("Respuesta recibida: %s\n", buffer);
    } else {
        printf("Error recibiendo respuesta. Código: %d\n", WSAGetLastError());
    }
}

int main(void) {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    int option;

    // Inicializar Winsock
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
        printf("1. Conectar al ESP32 y leer un valor\n");
        printf("2. Cerrar el programa\n");
        printf("Elige una opción: ");
        scanf("%d", &option);

        if (option == 1) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Llamar a la función para conectar y leer un valor del ESP32
            connect_to_esp32(&sock, &server);
            closesocket(sock);

        } else if (option == 2) {
            WSACleanup();
            printf("Conexión cerrada. Programa terminado.\n");
            break;
        } else {
            printf("Opción inválida, por favor elige otra.\n");
        }
    }

    return 0;
}
