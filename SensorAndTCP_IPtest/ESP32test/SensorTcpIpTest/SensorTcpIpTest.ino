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

int readAngle() {
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

int calculateAngle(int rawAngle) {
    // Convertir el valor crudo a un ángulo en grados
    return map(rawAngle, 0, 4095, 0, 360);
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
        bool sendData = true; // Variable para controlar si enviamos datos

        while (client.connected()) {
            if (client.available()) {
                // Leer mensaje del cliente
                String message = client.readStringUntil('\n');
                message.trim();

                if (message.equals("STOP")) {
                    sendData = false;
                    break;
                }
            }

            if (sendData) {
                // Leer y calcular el ángulo
                int rawAngle = readAngle();
                int angle = calculateAngle(rawAngle);

                // Determinar si el giro es positivo o negativo
                String angleMessage = "E0: ";
                angleMessage += String(angle);

                // Enviar mensaje al cliente
                client.print(angleMessage);

                delay(500); // Actualización cada 500 ms
            }
        }
        client.stop(); // Finalizar conexión
    }
}
