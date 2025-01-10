/**
  * This is a program to test the TCP/IP protocol
  * with an AS5600 sensor, reading the output of a
  * stepper motor, adding some usefull features.
  * Carlos Paredes Márquez, 10/01/2025.
  */

#include <Wire.h>
#include <UIPEthernet.h>

// Ethernet Server Configuration.
EthernetServer server(12345);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 192);

// Direction of the AS5600 sensor.
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_HIGH 0x0C
#define RAW_ANGLE_LOW  0x0D

// Global variable of the zero position.
float zeroPosition = 0.0;

/**
  * This function configure the ESP32 and establish the communication
  * bases. It start's the Ethernet connection using the MAC address
  * and the static IP address. This allows the ESP32 to act as a server
  * on the local network. And the I2C allows the ESP32 to communicate
  * with the sensor to read angle data.
  */

void setup() {
    Ethernet.begin(mac, ip);  //  Ethernet config. in the port: 12345
    server.begin();           //  Server init.
    Wire.begin(21, 22);       //  I2C init.
}

/**
  * This function reads the ADC (12 bits) input of the AS5600 sensor.
  * And returns the raw angle converted into float degree's.
  * @Wire.beginTransmission(AS5600_ADDRESS); Initiates a transmission
  * to the sensor, using its I2C address (0x36).
  * @Wire.write(RAW_ANGLE_HIGH); Send the address of the data register
  * indicating that we want to read the angle value.
  * @Wire.endTransmission(); Ends the write transmission.
  * @Wire.requestFrom(AS5600_ADDRESS, 2); Request 2 bytes of data from 
  * the sensor: The high value and the low value of the angle.
  * @return (rawAngle / 4095.0) * 360.0; Convert raw value to angle 
  * in degrees.
  */

float readRawAngle() {
    Wire.beginTransmission(AS5600_ADDRESS);
    Wire.write(RAW_ANGLE_HIGH);
    Wire.endTransmission();
    Wire.requestFrom(AS5600_ADDRESS, 2);

    /**
      * The AS5600 sensor stores the angle values in two consecutive
      * 8-bit registers. 
      * @RAW_ANGLE_HIGH (0x0C): Contains the 8 most significant bits.
      * @RAW_ANGLE_LOW (0x0D): Contains the 8 least significant bits.
      **/

    int highByte = Wire.read();
    int lowByte = Wire.read();

    /**
      * int rawAngle = (highByte << 8) | lowByte;
      * Combines the two received byted into a 16-bit integer
      * to form the raw angle value.
      * highByte << 8: Shift the highByte bits 8 position
      * to the left.
      */
    int rawAngle = (highByte << 8) | lowByte;

    return (rawAngle / 4095.0) * 360.0;
}

/**
  * This function is intented to read the angle from the 
  * AS5600 sensor and adjust this value according to a "zero point"
  * (previous calibration).
  * @angle: Read the angle and subtract the initial position.
  * The calculared angle may be outside the standard range 0° to 
  * 360° after subtraction. Therefore, it is normalized.
  * Example:
  *   -30 + 360 = 330.
  *   370 - 360 = 10.
  * @return angle: returns the final value of the angle, already 
  * calibrated and normalized within the range 0° and 360°.
  */

float readCalibratedAngle() {
    float angle = readRawAngle() - zeroPosition;

    // Normalize the anglo to the range 0° to 360°.
    if (angle < 0) angle += 360.0;
    if (angle >= 360) angle -= 360.0;

    return angle;
}

/**
  * This function restores the current sensor position as the
  * new "zero" or reference point.
  * @readRawAngle(); reads the current raw angle of the sensor
  * without any correction or calibration. This value is assigned
  * to the gloabl variable @zeroPosition.
  * @zeroPosition is used in the readCalibratedAngle() function
  * to the new zero point.
  */

void resetPosition() {
    zeroPosition = readRawAngle();
}

void loop() {
    EthernetClient client = server.available();

    if (client) {
        while (client.connected()) {
            if (client.available()) {
                String message = client.readStringUntil('\n');
                message.trim();

                if (message.equals("RESET")) {
                    resetPosition();
                    client.println("Position reset to 0.0");

                } else if (message.equals("READ")) {
                    float angle = readCalibratedAngle();
                    client.println(angle, 2);

                } else if (message.equals("START")) {
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
