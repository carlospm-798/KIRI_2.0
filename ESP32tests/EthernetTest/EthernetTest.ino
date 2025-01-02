#include <EthernetENC.h>
#include <SPI.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Dirección MAC

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera a que se abra el puerto serie
  }

  Ethernet.init(5); // Cambia el pin CS si es necesario
  Ethernet.begin(mac);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No se encontró hardware Ethernet.");
  } else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("El cable Ethernet no está conectado.");
  } else {
    Serial.println("Conexión Ethernet establecida.");
  }
}

void loop() {
  // Aquí puedes añadir tu lógica
}
