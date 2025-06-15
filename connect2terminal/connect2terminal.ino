#include <SPI.h>
#include <UIPEthernet.h>
#include <Wire.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,100), gw(192,168,0,1), nm(255,255,255,0);
EthernetServer server(5000);

float angleOffset = 0.0f;
int turns = 0;

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

float getAngle() {
    float raw = readAS5600() * 360.0f / 4096.0f;
    return raw;
}

float getCorrectedAngle() {
    float corrected = getAngle() - angleOffset;
    if (corrected < 0) corrected += 360;
    else if (corrected >= 360) corrected -= 360;

    return corrected;
}

bool getDirection(float current, float last) {
    if (current > last) return true;
    else if (current < last) return false;
    return 0;
}

void displayAngle(float corrected, bool direction, int turns) {
    float corrected_with_turns = corrected + (turns * 360.0f);

    return corrected_with_turns;
}

class SensorHandler {
private:
    const int addr = 0x36; // AS5600 I2C address

public:
    void begin() {
        Wire.begin(21, 22);
    }

    uint16_t readSensor() {
        Wire.beginTransmission(addr);
        Wire.write(0x0E);
        if (Wire.endTransmission(false) != 0) return 0;
        Wire.requestFrom(addr, 2);
        if (Wire.available() < 2) return 0;
        uint8_t h = Wire.read(), l = Wire.read();
        return ((h << 8) | l) & 0x0FFF;
    }
};

SensorHandler sensor;

void setup() {
    sensor.begin();
    SPI.begin(18, 19, 23, 5);
    Ethernet.begin(mac, ip, gw, nm);
    server.begin();
}

void loop() {
    EthernetClient client = server.available();
    if (!client) return;

    float last_corrected = getCorrectedAngle()

    while (client.connected()) {

        float corrected = getCorrectedAngle();
        bool Direction = getDirection(corrected, last_corrected);

        // WORK IN PROGRESS



        if (client.available()) {
            char cmd = client.read();

            if (cmd == '1') {
                float angle = getCorrectedAngle();
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
