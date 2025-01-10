#include <Wire.h>
#include <UIPEthernet.h>

// Configuración del servidor Ethernet
EthernetServer server(12345);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 192);

// Dirección del sensor AS5600
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_HIGH 0x0C
#define RAW_ANGLE_LOW  0x0D

// Variable global para el desplazamiento (offset)
float zeroPosition = 0.0;

void setup() {
    Ethernet.begin(mac, ip);
    server.begin();
    Wire.begin(21, 22); // Pines SDA = 21, SCL = 22
}

float readRawAngle() {
    // Leer los registros del sensor
    Wire.beginTransmission(AS5600_ADDRESS);
    Wire.write(RAW_ANGLE_HIGH);
    Wire.endTransmission();
    Wire.requestFrom(AS5600_ADDRESS, 2);

    int highByte = Wire.read();
    int lowByte = Wire.read();
    int rawAngle = (highByte << 8) | lowByte;

    // Convertir el valor crudo a un ángulo en grados
    return (rawAngle / 4095.0) * 360.0;
}

float readCalibratedAngle() {
    // Leer el ángulo crudo y restar la posición inicial
    float angle = readRawAngle() - zeroPosition;

    // Normalizar el ángulo al rango de 0° a 360°
    if (angle < 0) angle += 360.0;
    if (angle >= 360) angle -= 360.0;

    return angle;
}

void resetPosition() {
    zeroPosition = readRawAngle();  // Establecer la posición actual como el nuevo 0°
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
        while (client.connected()) {
            if (client.available()) {
                String message = client.readStringUntil('\n');
                message.trim();

                if (message.equals("RESET")) {
                    // Establecer la posición actual como 0°
                    resetPosition();
                    client.println("Position reset to 0.0");
                } else if (message.equals("READ")) {
                    // Enviar una única lectura
                    float angle = readCalibratedAngle();
                    client.println(angle, 2); // 2 decimales de precisión
                } else if (message.equals("START")) {
                    // Enviar lecturas continuamente
                    while (client.connected()) {
                        float angle = readCalibratedAngle();
                        client.println(angle, 2);
                        if (client.available()) {
                            String stopMessage = client.readStringUntil('\n');
                            stopMessage.trim();
                            if (stopMessage.equals("STOP")) break;
                        }
                    }
                }
            }
        }
        client.stop();
    }
}
