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

#define STEP_DELAY_US 250

// PID params
float Kp = 1.2f, Ki = 0.05f, Kd = 0.3f;
float targetAngle = 0.0f;
float speedPercent = 50.0f;

int currentTurns = 0;
float prevAngle = 0.0f;

float prevError = 0.0f, integral = 0.0f;
float prevFilteredOutput = 0.0f;
unsigned long prevTime = 0;

EthernetClient client;

void setup() {
  Wire.begin(21, 22);
  SPI.begin(18, 19, 23, 5);
  Ethernet.begin(mac, ip, gw, nm);
  server.begin();

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);

  prevAngle = readAS5600();
  prevTime = micros();
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

void doStep(bool forward) {
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_DELAY_US / 2);
}

void loop() {
  if (!client || !client.connected()) {
    client = server.available();
    return;
  }

  // Receive target update
  if (client.available()) {
    String cmd = client.readStringUntil('\n');
    if (cmd.startsWith("TARGET:")) {
      int first = cmd.indexOf(':');
      int second = cmd.indexOf(':', first + 1);
      if (second > 0) {
        float target = cmd.substring(first + 1, second).toFloat();
        float speed = cmd.substring(second + 1).toFloat();

        targetAngle = target; // Guardar target como ángulo puro (0° - 360°)
        speedPercent = speed;

        // Reset PID
        prevFilteredOutput = 0.0f;
        integral = 0.0f;
        prevError = 0.0f;
        prevTime = micros();
      }
    }
    else if (cmd == "0") {
      client.stop();
      return;
    }
  }

  // PID control loop
  float angle = readAS5600() * 360.0f / 4096.0f;
  float delta = angle - prevAngle;
  if (delta > 180.0f) currentTurns -= 1;
  else if (delta < -180.0f) currentTurns += 1;
  prevAngle = angle;

  float currentGlobal = currentTurns * 360.0f + angle;

  // Calcular p mod 360
  float p_mod360 = fmod(currentGlobal, 360.0f);
  if (p_mod360 < 0) p_mod360 += 360.0f;

  // Calcular error e = pd - p
  float error = targetAngle - p_mod360;
  if (error > 180.0f) error -= 360.0f;
  if (error < -180.0f) error += 360.0f;

  integral += error;
  integral = constrain(integral, -1000.0f, 1000.0f);

  float derivative = error - prevError;
  prevError = error;

  float output = Kp * error + Ki * integral + Kd * derivative;

  float alpha = 0.2f;
  float filteredOutput = alpha * output + (1.0f - alpha) * prevFilteredOutput;

  unsigned long now = micros();
  float dt = (now - prevTime) / 1000000.0f;
  prevTime = now;

  float accelLimit = 1000.0f;
  float deltaOutput = filteredOutput - prevFilteredOutput;
  float maxDelta = accelLimit * dt;
  deltaOutput = constrain(deltaOutput, -maxDelta, maxDelta);

  filteredOutput = prevFilteredOutput + deltaOutput;
  prevFilteredOutput = filteredOutput;

  float maxDegPerSec = 5000.0f * (speedPercent / 100.0f);
  filteredOutput = constrain(filteredOutput, -maxDegPerSec, maxDegPerSec);

  float degPerStep = 360.0f / (200.0f * 8.0f);
  int steps = int(abs(filteredOutput) * dt / degPerStep);
  steps = constrain(steps, 0, 30);

  if (steps > 0 || abs(error) > 0.1f) { // Tolerancia reducida a 0.1°
    if (steps == 0) steps = 1;
    bool forward = (filteredOutput > 0);
    for (int i = 0; i < steps; ++i) {
      doStep(forward);
    }
  }
}
