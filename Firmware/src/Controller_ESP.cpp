#include <esp_now.h>
#include <WiFi.h>

#define PIN_VRX_LEFT 35
#define PIN_VRY_LEFT 34

// REPLACE WITH YOUR RECEIVER MAC Address
//6c:c8:40:4e:93:6c
uint8_t broadcastAddress[] = {0x6c, 0xc8, 0x40, 0x4e, 0x93, 0x6c};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int x_L;
  int y_L;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Read analog values from joystick (0 to 4095)
  myData.x_L = analogRead(PIN_VRX_LEFT);
  myData.y_L = analogRead(PIN_VRY_LEFT);

  // Print values locally for verification
  Serial.print("Sending -> X: ");
  Serial.print(myData.x_L);
  Serial.print(" | Y: ");
  Serial.println(myData.y_L);

  // Send joystick data via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

  // Adjusted loop rate (50ms = 20Hz update rate)
  delay(50);
}