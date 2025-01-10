#include <stdio.h>          // Librería de entrada/salida
#include <winsock2.h>       // Librería de sockets y red
#include <string.h>         // Librería de cadenas
#include <windows.h>        // Librería para controlar el tiempo y la pantalla

#pragma comment(lib, "ws2_32.lib") // Enlace con la librería Winsock

#define PORT 12345            // Puerto del servidor ESP32
#define ESP32_IP "192.168.0.192" // Dirección IP fija del ESP32

// Variable global estática para almacenar el valor anterior
static char previous_buffer[1024] = {0}; // Buffer para el valor anterior

void connect_to_esp32(SOCKET* sock, struct sockaddr_in* server, const char* message) {
    char buffer[1024] = {0}; // Buffer para almacenar respuesta

    // Conectar al servidor
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectado al servidor.\n");

    // Enviar el mensaje al ESP32
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

// Función para leer datos continuamente
void read_continuous_data(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0};
    char message[] = "START\n"; // Mensaje para empezar a recibir datos continuamente
    DWORD start_time = GetTickCount(); // Hora de inicio de la lectura

    // Conectar al servidor
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectado al servidor para lectura continua.\n");

    // Enviar el comando "START" al ESP32
    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error enviando datos. Código: %d\n", WSAGetLastError());
        return;
    }

    while (GetTickCount() - start_time < 10000) { // 5000 ms = 5 segundos
        // Leer la respuesta del ESP32
        int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0'; // Asegurar que el buffer sea una cadena válida

            //system("cls");
            printf("Pos: %s", buffer);

            // Llamar a la función para imprimir los nuevos datos si es diferente al anterior
            /*if (buffer != previous_buffer) { // Si son diferentes
                // Limpiar la pantalla
                system("cls");
                // Imprimir el nuevo dato
                printf("Respuesta recibida: %s\n", buffer);
                // Actualizar el valor anterior con el nuevo dato
                strcpy(previous_buffer, buffer);
            }*/

        } else {
            printf("Error recibiendo respuesta. Código: %d\n", WSAGetLastError());
        }
    }

    printf("\n\nLectura continua completada.\n");
}

void send_reset_command(SOCKET* sock, struct sockaddr_in* server) {
    char message[] = "RESET\n";
    char buffer[1024] = {0};

    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error conectando al servidor. Código: %d\n", WSAGetLastError());
        return;
    }
    printf("Conectando al servidor para resetear la posición.\n");

    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error enviando datos. Código: %d\n", WSAGetLastError());
        return;
    }

    // Leer la respuesta del servidor
    int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("Respuesta del servidor: %s\n", buffer);
    } else {
        printf("error recibiendo respuesta. Código: %d\n", WSAGetLastError());
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
        printf("2. Conectar al ESP32 y leer datos continuamente por 10 segundos\n");
        printf("3. Resetear la posición a 0\n");
        printf("4. Cerrar el programa\n");
        printf(": ");
        scanf("%d", &option);

        if (option == 1) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Llamar a la función para conectar y leer un valor del ESP32
            connect_to_esp32(&sock, &server, "READ\n");
            closesocket(sock);

        } else if (option == 2) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Llamar a la función para lectura continua
            read_continuous_data(&sock, &server);
            closesocket(sock);

        } else if (option == 3) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creando socket. Código: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }
            send_reset_command(&sock, &server);
            closesocket(sock);

        } else if (option == 4) {
            WSACleanup();
            printf("Conexión cerrada. Programa terminado.\n");
            break;
        } else {
            printf("Opción inválida, por favor elige otra.\n");
        }
    }

    return 0;
}
