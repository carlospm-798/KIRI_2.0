#include <SPI.h>
#include <UIPEthernet.h>
#include <Wire.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,100), gw(192,168,0,1), nm(255,255,255,0);
EthernetServer server(5000);

void setup() {
  Wire.begin(21, 22);
  SPI.begin(18, 19, 23, 5);
  Ethernet.begin(mac, ip, gw, nm);
  server.begin();
}

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

void loop() {
  EthernetClient client = server.available();
  if (!client) return;

  while (client.connected()) {
    if (client.available() > 0) {
      char cmd = client.read();
      if (cmd == '1') {
        uint16_t angle = readAS5600();
        client.print(angle);
      }
      else if (cmd == '0') {
        client.stop();
        break;
      }
    }
    yield(); // en lugar de delay
  }
}

