#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "MPU6050.h"

#define ESC_PIN 13

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

// Data structure matching the sender
typedef struct struct_message {
  int x_L;
  int y_L;
} struct_message;

// Shared volatile structure updated by ESP-NOW callback
volatile struct_message receivedJoyData = {2048, 2048};

Servo esc;

// ESP-NOW Receive Callback (Runs asynchronously)
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_message)) {
    memcpy((void*)&receivedJoyData, incomingData, sizeof(struct_message));
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Initialize I2C bus at 400kHz for fast sensor reads
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // 2. Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed! Check wiring on GPIO 21/22.");
    while (1);
  }
  Serial.println("MPU6050 connected.");

  // 3. Initialize ESC on GPIO 13
  ESP32PWM::allocateTimer(0);
  esc.setPeriodHertz(50);
  esc.attach(ESC_PIN, 1000, 2000);
  esc.writeMicroseconds(1000); // 1000us arming pulse

  // 4. Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  lastLoopTime = micros();
  Serial.println("Flight controller initialized.");
}

void loop() {
  // Compute delta time (dt in seconds)
  unsigned long currentTime = micros();
  float dt = (currentTime - lastLoopTime) / 1000000.0;
  lastLoopTime = currentTime;

  // 1. Read Raw 6-Axis Motion Data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 2. Convert Gyro rates to deg/sec (default range +/- 250 deg/s -> 131 LSB/(deg/s))
  float gyroRateX = gx / 131.0;
  float gyroRateY = gy / 131.0;

  // 3. Compute Accelerometer Pitch & Roll in degrees
  // roll = rotation around X-axis; pitch = rotation around Y-axis
  float accelRoll  = atan2(ay, az) * (180.0 / PI);
  float accelPitch = atan2(-ax, sqrt((float)ay * ay + (float)az * az)) * (180.0 / PI);

  // 4. Complementary Filter
  // 98% Gyro integration (fast, eliminates vibration noise) + 2% Accel (eliminates long-term drift)
  roll  = 0.98 * (roll + gyroRateX * dt) + 0.02 * accelRoll;
  pitch = 0.98 * (pitch + gyroRateY * dt) + 0.02 * accelPitch;

  // 5. Read joystick data passed from ESP-NOW
  int throttleCmd = receivedJoyData.y_L; // 0 to 4095
  int motorPulse = map(throttleCmd, 100, 4095, 1000, 2000);
  motorPulse = constrain(motorPulse, 1000, 2000);
  esc.writeMicroseconds(motorPulse);

  // Print telemetry every ~100ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();
    Serial.printf("Pitch: %6.2f | Roll: %6.2f | JoyY: %4d | Motor Pulse: %4d us\n",
                  pitch, roll, throttleCmd, motorPulse);
  }

  // Target loop rate: ~250 Hz (4000 microseconds)
  while (micros() - currentTime < 4000);
}