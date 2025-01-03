/*  This code works as a client connected to the server
    (ESP32) to send and receive data.   
    */

#include <stdio.h>          // Input/Output library
#include <winsock2.h>       // Sockets and net library
#include <string.h>         // Strings library

/*  This command tells to the compiler that it should link
    the library ws2_32.lib, which is the librery of Winsock
    of Windows. This help us to use the defined net functions
    in winsock2.h.     */
#pragma comment(lib, "ws2_32.lib")

#define PORT 12345      // define the port 12345 of the server (ESP32)
#define ESP32_IP "192.168.0.192"  // fixed IP of the ESP32




void connect_to_esp32(SOCKET *sock, struct sockaddr_in *server) {
    char buffer[1024] = {0};    // buffer that saves the server answer
    char message = 'A';  // This is the message to the server

    /*  connect(): This function try's to get a connection to the server in the
        direction and port that where specify. If the connection fails, it returns a
        negative value.    */
    if (connect(*sock, (struct sockaddr*)server, sizeof(*server)) < 0) {
        printf("Error connecting to server. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Connected to server.\n");

    /*  send(): Sends data over the socker. If something wrong happend, the function
    return a negative value.    */
    if (send(*sock, &message, sizeof(message), 0) < 0) {
        printf("Error sending data. Code: %d\n", WSAGetLastError());
        return;
    }
    printf("Message sent: %c\n", message);

    /*  recv(): This function receive data of the server. If something wrong happend, the
        function return a negative value. If the data is right, it would be saved in a buffer.  
        */
    int valread = recv(*sock, buffer, sizeof(buffer), 0);
    if (valread > 0) {
        buffer[valread] = 0;
        printf("Response received: %s\n", buffer);
    } else {
        printf("Error receiving response. Code: %d\n", WSAGetLastError());
    }
}



int main(void) {
    
    WSADATA wsa;    // Structure to save the data of initialization of Winsock
    SOCKET sock;    // Represent the socket that would be used the TCP connection
    struct sockaddr_in server;  // Structure the contains the server direction
    int option;     // Variable of the menu option

    
    /*  This function initializes the Winsock librery in Windows.
        Is the first step to work with nets i C. If the response is 0, 
        the initialization is right.    
        
        MAKEWORD(2, 2): Specifies that we are using the version 2.2 of Winsock
        
        &wsa: Is the direction of memory of the structure wsa. This is necessary
        because WSAStartup() needs to access to this structure to fill out relevant
        information of the Winsock initialization.
        */

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error initializing Winsock. Code: %d\n", WSAGetLastError());
        return 1;
    }

    /*  memset(): Is a standard function of C, that is used to set 
        a block of memory a specific value.
        
            void *memset(void *ptr, int value, size_t num);

            ptr: Is the pointer of the memory block that we want to modify
            value: Is the value that we want to fill the block of memory
            num: Is the number of bytes that we want to modify

        (char*)&server: We are taking the memory direction of the variable SERVER, changing it
        to a pointer type char. This is necessary becouse memset() works with bytes.
        0: Is the value which we want to fill the memory of the structure SERVER. 
        sizeof(server): This determines how many bytes the server structure memory will occupy.
        */
    memset((char*)&server, 0, sizeof(server));
    
    server.sin_family = AF_INET;    // sin_family: defines the type of direction; IPv4 (AF_INET).
    server.sin_port = htons(PORT);  // sin_port: defines the port that we want to connect.
    server.sin_addr.s_addr = inet_addr(ESP32_IP);   // Is the IP of the server to which we want to connect.



    while (1) {
        
        printf("\nOptions:\n");
        printf("1. Connect to the ESP32 and receive data\n");
        printf("2. Close the program\n");
        printf("Type option: ");
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
            /*  closesocket(): Closed the socked once the communication ends.
                WSACleanup(): Free Winsock resources
                */
            closesocket(sock);
            WSACleanup();

            printf("Connection closed. Program terminated..\n");
            break;
        } else {
            printf("Invalid option, please choose again.\n");
        }

        Sleep(1000);
    }
    return 0;
}
