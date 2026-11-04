#include <esp_now.h>
#include <WiFi.h>

// Left Joystick (ADC1)
#define PIN_VRX_LEFT  35
#define PIN_VRY_LEFT  34

// Right Joystick (ADC1)
#define PIN_VRX_RIGHT 33
#define PIN_VRY_RIGHT 32

// Receiver MAC Address
uint8_t broadcastAddress[] = {0x6c, 0xc8, 0x40, 0x4e, 0x93, 0x6c};

// Updated structure containing both joysticks
typedef struct struct_message {
  int x_L;
  int y_L;
  int x_R;
  int y_R;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nPacket Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Read Left Joystick
  myData.x_L = analogRead(PIN_VRX_LEFT);
  myData.y_L = analogRead(PIN_VRY_LEFT);

  // Read Right Joystick
  myData.x_R = analogRead(PIN_VRX_RIGHT);
  myData.y_R = analogRead(PIN_VRY_RIGHT);

  // Local debug print
  Serial.printf("L: [%4d, %4d] | R: [%4d, %4d]\n", 
                myData.x_L, myData.y_L, myData.x_R, myData.y_R);

  // Send packet
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result != ESP_OK) {
    Serial.println("Error initiating send");
  }

  // 20Hz update rate
  delay(50);
}