#include <UIPEthernet.h>

// Definir MAC address única
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Iniciando Ethernet...");

  // Inicializar Ethernet con DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Fallo al inicializar Ethernet con DHCP. Verifica conexiones.");
    while (true);
  }

  // Imprimir IP asignada
  Serial.print("IP asignada: ");
  Serial.println(Ethernet.localIP());

  // Iniciar servidor web
  server.begin();
  Serial.println("Servidor iniciado.");
}

void loop() {
  // Esperar solicitudes de cliente
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Cliente conectado.");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1>Hola Mundo desde ESP32</h1>");
    client.println("</html>");
    delay(1);
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}
