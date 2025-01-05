#include <Wire.h>          // Librería para comunicación I2C
#include <UIPEthernet.h>   // Librería para el módulo Ethernet ENC28J60

// Configuración del servidor Ethernet
EthernetServer server(12345);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 192);

// Dirección del sensor AS5600
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_HIGH 0x0C
#define RAW_ANGLE_LOW  0x0D

void setup() {
    // Iniciar Ethernet
    Ethernet.begin(mac, ip);
    server.begin();

    // Iniciar I2C
    Wire.begin(21, 22); // Pines SDA = 21, SCL = 22
}

int readRawAngle() {
    // Leer los registros del sensor
    Wire.beginTransmission(AS5600_ADDRESS);
    Wire.write(RAW_ANGLE_HIGH);
    Wire.endTransmission();
    Wire.requestFrom(AS5600_ADDRESS, 2);

    int highByte = Wire.read();
    int lowByte = Wire.read();
    int rawAngle = (highByte << 8) | lowByte;

    return rawAngle;
}

float calculateAngle(int rawAngle) {
    // Convertir el valor crudo a un ángulo flotante en grados
    return (rawAngle * 360.0f) / 4096.0f;
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
        bool sendData = false; // Controla si estamos en modo continuo

        while (client.connected()) {
            if (client.available()) {
                // Leer mensaje del cliente
                String message = client.readStringUntil('\n');
                message.trim();

                if (message.equals("READ")) {
                    // Leer y enviar un solo dato
                    int rawAngle = readRawAngle();
                    float angle = calculateAngle(rawAngle);

                    String angleMessage = "E0: ";
                    angleMessage += String(angle, 2); // 2 decimales
                    client.println(angleMessage);

                } else if (message.equals("START")) {
                    // Activar modo continuo
                    sendData = true;

                } else if (message.equals("STOP")) {
                    // Desactivar modo continuo
                    sendData = false;
                }
            }

            if (sendData) {
                // Enviar datos continuamente
                int rawAngle = readRawAngle();
                float angle = calculateAngle(rawAngle);

                String angleMessage = "E0: ";
                angleMessage += String(angle, 2); // 2 decimales
                client.println(angleMessage);
            }
        }

        client.stop(); // Finalizar conexión
    }
}
