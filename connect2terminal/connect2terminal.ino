#include <SPI.h>
#include <UIPEthernet.h>
#include <Wire.h>

// MAC and static IP
byte mac[]    = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };
IPAddress ip(192,168,0,100), gw(192,168,0,1), nm(255,255,255,0);
EthernetServer server(5000);

// Unused GPIOs to drive LOW (avoid floating)
const int unusedPins[] = { 2,4,12,13,14,15,25,26,27 };

// AS5600 I2C address
const uint8_t AS5600_ADDR = 0x36;

void setup() {
  Serial.begin(115200);

  // Drive unused pins LOW
  for (int p : unusedPins) {
    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  // Explicit SPI init for ENC28J60 (VSPI)
  // SCK=18, MISO=19, MOSI=23, SS=5
  SPI.begin(18, 19, 23, 5);

  // I2C init: SDA=21, SCL=22
  Wire.begin(21, 22);

  // Ethernet init with static IP
  Ethernet.begin(mac, ip, gw, nm);
  server.begin();
  Serial.print("Server listening on ");
  Serial.print(ip);
  Serial.println(":5000");
}

// Read raw 12-bit angle (0–4095) from AS5600
uint16_t readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0E);               // Address of angle MSB
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() < 2) return 0;
  uint8_t high = Wire.read(), low = Wire.read();
  return ((high << 8) | low) & 0x0FFF;
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    // Keep connection open until client sends '0'
    while (client.connected()) {
      if (client.available()) {
        char cmd = client.read();
        if (cmd == '1') {
          // Send sensor reading
          uint16_t angle = readAS5600();
          client.print(angle);
        }
        else if (cmd == '0') {
          // Client requested disconnect
          client.stop();
          Serial.println("Client disconnected by command");
          break;
        }
      }
      delay(1);
    }
  }
}
