/**
 * @author Carlos Paredes Márquez
 * @date Jan. 12 2025
 * This code works as an intermediary between 
 * the esp32 as a server, through the nc28j60 
 * ethernet module.
*/


#include <stdio.h>          // Input/Output library
#include <winsock2.h>       // Sockets and network library
#include <string.h>         // String library

#pragma comment(lib, "ws2_32.lib") // Connection to Winsock library

#define PORT 12345            // Server port of the ESP32
#define ESP32_IP "192.168.0.192" // Fix IP of the ESP32

static char previous_buffer[1024] = {0}; // Static global variable to store the previous value

/**
 * Common parameters between functions:
 * 
 * @param sock Pointer to the socket for communication with the server.
 * @param server Structure containing the IP address and port of the ESP32 server.
 * 
 * @details
 * - The function establishes a connection to the ESP32 using the data in `server`.
 * - Sends the message specified in `message`.
 * - Receives and displays the response from the ESP32.
 * 
 * @note
 * - In case of errors, the error code is printed using `WSAGetLastError`.
 * - Requires the socket to be properly initialized.
 *
 * @example
 * 
 * SOCKET* sock: This is a pointer to an object of type SOCKET.
 * The socket is the channel through which data is sent and received between
 * the cient (C) and the server (ESP32).
 * 
 * struct sockaddr_in* server: This is a pointer to a structure of type
 * sockaddr_in. It defines the properties of the server that the client will
 * attempt to connect to:
 * 
 * sin_family: Address type (indicating IPv4).
 * sin_port: Server port (12345).
 * sin_addr.s_addr: Server IP address (192.168.0.192).
 * 
 * const char* message: This is a constant pointer to a string containing the
 * message to be sent to the ESP32 when establishing the connection. For example
 * commands like "READ\n" or "START\n".
 */

/**
 * This function attempts to connect the socket to the ESP32
 * using the provided address and port. Sends a message to the ESP32.
 * Receives the response from the ESP32 and prints it. And Handle errors
 * in the connection, sending and receiving phases.
*/

void connect_to_esp32(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0}; // Buffer to store response
    char message[] = "READ\n";

    // Connect to server
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error connecting to server. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Connected to server.\n");

    // Send the message to the ESP32
    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error sending data. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Message sent: %s", message);

    // Read the ESP32 response
    int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
    if (valread > 0) {
        buffer[valread] = '\0'; // Ensure buffer is a valid string
        printf("Response received: %s\n", buffer);
    } else {
        printf("Error receiving response. Code: %d\n", WSAGetLastError());
    }
}


/**
 * This function performs continuous reading of data sent from the
 * ESP32 for a specific period of time (10 seconds for example). Sends a message 
 * to the ESP32. Receives the response from the ESP32 and prints it. And Handle errors
 * in the connection, sending and receiving phases.
*/

void read_continuous_data(SOCKET* sock, struct sockaddr_in* server) {
    char buffer[1024] = {0};
    char message[] = "START\n"; // Message to start receiving data continuously
    DWORD start_time = GetTickCount(); // Reading start time

    // Connect to server
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error connecting to server. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Connected to server.\n");

    // Send the message to the ESP32
    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error sending data. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Message sent: %s", message);

    while (GetTickCount() - start_time < 10000) { // 1000 ms = 1 second
        // Read the ESP32 response
        int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0'; // Ensure buffer is a valid string

            printf("Pos: %s", buffer);

        } else {
            printf("Error receiving response. Code: %d\n", WSAGetLastError());
        }
    }

    printf("\n\nContinuous reading completed.\n");
}

/**
 * This function is used to reset the sensor position to 0° degrees. Sends a message to 
 * the ESP32. Receives the response from the ESP32 and prints it. And Handle errors
 * in the connection, sending and receiving phases.
*/

void send_reset_command(SOCKET* sock, struct sockaddr_in* server) {
    char message[] = "RESET\n";
    char buffer[1024] = {0};

    // Connect to server
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error connecting to server. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Connected to server.\n");

    // Send the message to the ESP32
    if (send(*sock, message, strlen(message), 0) < 0) {
        printf("Error sending data. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Message sent: %s", message);

    // Read the ESP32 response
    int valread = recv(*sock, buffer, sizeof(buffer) - 1, 0);
    if (valread > 0) {
        buffer[valread] = '\0'; // Ensure buffer is a valid string
        printf("Response received: %s\n", buffer);
    } else {
        printf("Error receiving response. Code: %d\n", WSAGetLastError());
    }
}


int main(void) {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    int option;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error initializing Winsock. Code: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(ESP32_IP);

    while (1) {
        printf("\nOptions:\n");
        printf("1. Connect to ESP32 and read a value\n");
        printf("2. Connect to ESP32 and read data continuously for 10 seconds\n");
        printf("3. Reset position to 0\n");
        printf("4. Close the program\n");
        printf(": ");
        scanf("%d", &option);

        if (option == 1) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creating socket. Code: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            connect_to_esp32(&sock, &server);
            closesocket(sock);

        } else if (option == 2) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creating socket. Code: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            read_continuous_data(&sock, &server);
            closesocket(sock);

        } else if (option == 3) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                printf("Error creating socket. Code: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }
            send_reset_command(&sock, &server);
            closesocket(sock);

        } else if (option == 4) {
            WSACleanup();
            printf("Connection closed. Program terminated.\n");
            break;
        } else {
            printf("Invalid option, please choose another one.\n");
        }
    }

    return 0;
}
