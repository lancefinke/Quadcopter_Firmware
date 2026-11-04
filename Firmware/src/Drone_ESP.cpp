#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "MPU6050.h"

// ESC Pin Definitions
#define ESC1_PIN 13  // Motor 1 (Left Stick Y - y_L)
#define ESC2_PIN 14  // Motor 2 (Left Stick X - x_L)
#define ESC3_PIN 12  // Motor 3 (Right Stick Y - y_R)

// MPU-6050 instance
MPU6050 mpu;

// Raw sensor buffers
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Orientation angles (degrees)
float pitch = 0.0;
float roll  = 0.0;

// Loop timing
unsigned long lastLoopTime = 0;

// Matched data structure from transmitter
typedef struct struct_message {
  int x_L;
  int y_L;
  int x_R;
  int y_R;
} struct_message;

// Shared volatile structure updated by ESP-NOW callback
volatile struct_message receivedJoyData = {2048, 2048, 2048, 2048};

// Servo objects for ESCs
Servo esc1;
Servo esc2;
Servo esc3;

// Deadband threshold to hold motors reliably at idle/off
const int DEAD_BAND_LOW = 100;

// ESP-NOW Receive Callback (PlatformIO / Core 2.x signature)
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_message)) {
    memcpy((void*)&receivedJoyData, incomingData, sizeof(struct_message));
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Initialize I2C bus (SDA: D21, SCL: D22) at 400kHz
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // 2. Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed! Check wiring on GPIO 21/22.");
    while (1);
  }
  Serial.println("MPU6050 connected.");

  // 3. Allocate PWM timers and attach ESCs
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);

  esc1.setPeriodHertz(50);
  esc1.attach(ESC1_PIN, 1000, 2000);
  esc1.writeMicroseconds(1000); // 1000us arm pulse for Motor 1

  esc2.setPeriodHertz(50);
  esc2.attach(ESC2_PIN, 1000, 2000);
  esc2.writeMicroseconds(1000); // 1000us arm pulse for Motor 2

  esc3.setPeriodHertz(50);
  esc3.attach(ESC3_PIN, 1000, 2000);
  esc3.writeMicroseconds(1000); // 1000us arm pulse for Motor 3

  // 4. Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  lastLoopTime = micros();
  Serial.println("Drone ready: M1(D13:y_L), M2(D14:x_L), M3(D27:y_R).");
}

void loop() {
  unsigned long currentTime = micros();
  float dt = (currentTime - lastLoopTime) / 1000000.0;
  lastLoopTime = currentTime;

  // 1. Read Raw 6-Axis Motion Data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 2. Gyroscope angular rate (deg/sec)
  float gyroRateX = gx / 131.0;
  float gyroRateY = gy / 131.0;

  // 3. Accelerometer angles (deg)
  float accelRoll  = atan2(ay, az) * (180.0 / PI);
  float accelPitch = atan2(-ax, sqrt((float)ay * ay + (float)az * az)) * (180.0 / PI);

  // 4. Complementary Filter
  roll  = 0.98 * (roll + gyroRateX * dt) + 0.02 * accelRoll;
  pitch = 0.98 * (pitch + gyroRateY * dt) + 0.02 * accelPitch;

  // 5. Read Joystick Commands
  int yLCmd = receivedJoyData.y_L; // Motor 1
  int xLCmd = receivedJoyData.x_L; // Motor 2
  int yRCmd = receivedJoyData.y_R; // Motor 3

  // Map inputs to standard 1000-2000us PWM
  int pulseMotor1 = (yLCmd <= DEAD_BAND_LOW) ? 1000 : map(yLCmd, DEAD_BAND_LOW, 4095, 1000, 2000);
  pulseMotor1 = constrain(pulseMotor1, 1000, 2000);
  esc1.writeMicroseconds(pulseMotor1);

  int pulseMotor2 = (xLCmd <= DEAD_BAND_LOW) ? 1000 : map(xLCmd, DEAD_BAND_LOW, 4095, 1000, 2000);
  pulseMotor2 = constrain(pulseMotor2, 1000, 2000);
  esc2.writeMicroseconds(pulseMotor2);

  int pulseMotor3 = (yRCmd <= DEAD_BAND_LOW) ? 1000 : map(yRCmd, DEAD_BAND_LOW, 4095, 1000, 2000);
  pulseMotor3 = constrain(pulseMotor3, 1000, 2000);
  esc3.writeMicroseconds(pulseMotor3);

  // Telemetry stream every ~100ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();
    Serial.printf("M1(y_L): %4dus | M2(x_L): %4dus | M3(y_R): %4dus | Pitch: %5.1f | Roll: %5.1f\n",
                  pulseMotor1, pulseMotor2, pulseMotor3, pitch, roll);
  }

  // Deterministic 250 Hz loop execution
  while (micros() - currentTime < 4000);
}