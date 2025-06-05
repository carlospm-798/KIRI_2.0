#include <SPI.h>
#include <UIPEthernet.h>
#include <Wire.h>

// Ethernet config
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,100), gw(192,168,0,1), nm(255,255,255,0);
EthernetServer server(5000);

// Motor pins
#define STEP_PIN 14
#define DIR_PIN  27

void setup() {
  Wire.begin(21, 22); // I2C pins for AS5600
  SPI.begin(18, 19, 23, 5); // ENC28J60 (SCK, MISO, MOSI, CS)
  Ethernet.begin(mac, ip, gw, nm);
  server.begin();

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
}

// Lee el ángulo crudo de 0 a 4095 del AS5600
uint16_t readAS5600() {
  Wire.beginTransmission(0x36);
  Wire.write(0x0E);
  if (Wire.endTransmission(false)) return 0;
  Wire.requestFrom(0x36, 2);
  if (Wire.available() < 2) return 0;
  uint8_t high = Wire.read();
  uint8_t low  = Wire.read();
  return ((high << 8) | low) & 0x0FFF;
}

// Ejecuta un microstep en la dirección indicada
void doStep(bool forward) {
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(500);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(500);
}

void loop() {
  EthernetClient client = server.available();
  if (!client) return;

  while (client.connected()) {
    if (client.available()) {
      char cmd = client.read();

      if (cmd == '1') {
        uint16_t angle = readAS5600();
        client.print(angle);
      }
      else if (cmd == 'F') {
        doStep(true);  // paso hacia adelante
      }
      else if (cmd == 'B') {
        doStep(false); // paso hacia atrás
      }
      else if (cmd == '0') {
        client.stop();
        break;
      }
    }
    yield(); // evita bloqueo del loop
  }
}
