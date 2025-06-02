#include <SPI.h>
#include <UIPEthernet.h>
#include <Wire.h>

byte mac[]    = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };
IPAddress ip(192,168,0,100), gw(192,168,0,1), nm(255,255,255,0);
EthernetServer server(5000);
const int AS5600_ADDR = 0x36;

uint16_t readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0E);
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() < 2) return 0;
  uint8_t h = Wire.read(), l = Wire.read();
  return ((h << 8) | l) & 0x0FFF;
}

void setup() {
  Wire.begin(21, 22);
  SPI.begin(18, 19, 23, 5);
  Ethernet.begin(mac, ip, gw, nm);
  server.begin();
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
      else if (cmd == '0') {
        client.stop();
        break;
      }
    }
    delay(1);
  }
}
