#include <UIPEthernet.h>

// Configuración del servidor
EthernetServer server(12345);

// Dirección MAC del módulo ENC28J60
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Dirección IP estática opcional
IPAddress ip(192, 168, 1, 100);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Iniciando servidor...");

    // Intentar asignar IP automáticamente (DHCP)
    if (Ethernet.begin(mac) == 0) {
        Serial.println("No se pudo obtener una IP por DHCP. Usando IP estática...");
        Ethernet.begin(mac, ip); // Asignar IP estática
    }

    // Mostrar la IP asignada
    Serial.print("Dirección IP asignada: ");
    Serial.println(Ethernet.localIP());

    // Iniciar servidor
    server.begin();
    Serial.println("Servidor iniciado.");
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
        Serial.println("Cliente conectado.");
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.print("Recibido: ");
                Serial.println(c);

                // Responder al cliente
                client.write("ACK\n");
            }
        }
        client.stop();
        Serial.println("Cliente desconectado.");
    }
}
