#include <UIPEthernet.h>  // Ethernet library for the module ENC28J60.

// Creation of the server.
// The server would read the conexions of the port 12345.
EthernetServer server(12345);

// MAC direction.
// Media Access Control. This direction is unique for every
// device, and it's necessary to be identify in the net.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// fixed IP address.
// An IP is assigned to not depend of a DHCP server, this
// IP would be always the same, so it simplifies the connection
// with the C client.
IPAddress ip(192, 168, 0, 192);

void setup() {
    // Begin of the function Ethernet with the IP and MAC direction.
    Ethernet.begin(mac, ip);
    // Server starting, hearing the ethernet port.
    server.begin();
}

void loop() {
    // Checking if there's some client.
    // If yes, it return and EthernetClient object.
    EthernetClient client = server.available();

    if (client) {
        // Verify if the conection still
        while (client.connected()) {
            // Verify if there's some dava available
            if (client.available()) {
                // Reading and processing data
                char c = client.read();
                // Response to the client
                client.write("Read value: ");
                client.write(c);
            }
        }
        // Closing the conection
        client.stop();
    }
}
