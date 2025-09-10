#include <Arduino.h>
#include <esp_system.h>  
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <String.h>

// Function declarations
void wifiConnectionTask(void* pv);
void clientConnectionTask(void* pv);
void sendDataTask(void* pv);

const char* server = "server"; // Server URL
const char* path = "/path";

const char* ssid = "SSID";
const char* password = "PASSWORD";

// Global client made to share states between tasks.
WiFiClientSecure client;
SemaphoreHandle_t clientMutex;
void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  // Disables verification of SSL/TLS certificate. (for testing only).
  client.setInsecure();
  // provides time for ssl handshake.
  client.setTimeout(150000);
  // Create lock for contorlled access to global client.
  clientMutex = xSemaphoreCreateMutex();
  xTaskCreate(wifiConnectionTask, "wifiConnectionTask",10240,NULL,1,NULL);
  xTaskCreate(clientConnectionTask, "clientConnectionTask",10240,NULL,1,NULL);
  xTaskCreate(sendDataTask, "sendDataTask",10240,NULL,1,NULL);
}

void loop()
{
  //Empty loop function because the compiler requires it.
}

void wifiConnectionTask(void *pv){
  // This gives a more reliable DNS for the ESP32 to find the client-server.
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8,8,8,8), IPAddress(1,1,1,1));
  WiFi.begin(ssid, password);
  while(1){
      // // Wait for connection
    while (WiFi.status() != WL_CONNECTED) 
    {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Connecting to WiFi...");
    }

    Serial.println("Wifi Connected");
    vTaskDelete(NULL);
  }
}
// Helper function.
String modifyData(int* temperature, int* humidity) {
      // Dummy values (replace with sensor reading)
    *temperature = 30;
    *humidity = 70;

    String jsonData = "{"
      "\"temperature\":" + String(*temperature) + ", "
      "\"humidity\":" + String(*humidity) +
    "}";
    return jsonData;
}
void sendDataTask(void* pv){
    int temperature = 35;
    int humidity = 70;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(8000);

    while(1){
    // Create dependency, ensure client is connected before sending.
    if (xSemaphoreTake(clientMutex, portMAX_DELAY)){
      if (client.connected()){
      // Update atmospheric reading.
    String jsonData = modifyData(&temperature, &humidity);
    // Send HTTP POST
    client.println("POST " + String(path) + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println("Connection: close"); // closes the socket.connection.
    client.println();
    client.println(jsonData);
    // Read response
    String response = "";
    Serial.println("Data sent");
    // Close the connection, to prevent consequent data flow 
    // through dead SSL channels.
    client.stop();
  }
  xSemaphoreGive(clientMutex);
}
vTaskDelayUntil(&xLastWakeTime,xFrequency);
}
}

void clientConnectionTask(void* pv){
  TickType_t lastWakeTime = xTaskGetTickCount();
  while(1){
    if (xSemaphoreTake(clientMutex, portMAX_DELAY))
    {
      if(WiFi.status() == WL_CONNECTED){
        if (!client.connected())
        {
          // Clean up dead-connection.
          client.stop(); 
          if(client.connect(server, 443)) {
          Serial.println("Connected to server!");
          }
        }
      }
      xSemaphoreGive(clientMutex);
    }
    vTaskDelayUntil(&lastWakeTime, 5000 / portTICK_PERIOD_MS);
}
}