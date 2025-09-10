// #include <Arduino.h>
// #include <esp_system.h>  
// #include <WiFiClientSecure.h>
// #include <WiFi.h>
// #include <ArduinoJson.h>
// #include <String.h>

// const char* server = "server";       // Server URL
// const char* path = "/path";

// const char* ssid = "SSID";
// const char* password = "PASSWORD";

// // const char* ssid = "SSID2";
// // const char* password = "PASSWORD2";

// unsigned long lastSendTime = 0;
// const unsigned long sendInterval = 5000; // 5 seconds
// void setup()
// {
//     Serial.begin( 115200 ); /* prepare for possible serial debug */
//     WiFi.begin(ssid, password);
//       // // Wait for connection
//     while (WiFi.status() != WL_CONNECTED) 
//     {
//     delay(1000);
//     Serial.println("Connecting to WiFi...");
//     }

//     Serial.println("Wifi Connected");
    
//     WiFiClientSecure client;
//     client.setInsecure();
// }

// void loop() {
//   if (millis() - lastSendTime >= sendInterval) {
//     lastSendTime = millis();  // Update the timestamp

//     WiFiClientSecure client;
//     client.setInsecure(); // WARNING: Insecure connection (for testing only)

//     if (!client.connect(server, 443)) {
//       Serial.println("Connection to server failed!");
//       return;
//     }

//     // Dummy values (replace with sensor reading)
//     int temperature = 25;
//     int humidity = 60;

//     String jsonData = "{"
//       "\"temperature\":" + String(temperature) + ", "
//       "\"humidity\":" + String(humidity) +
//     "}";

//     // Send HTTP POST
//     client.println("POST " + String(path) + " HTTP/1.1");
//     client.println("Host: " + String(server));
//     client.println("Content-Type: application/json");
//     client.print("Content-Length: ");
//     client.println(jsonData.length());
//     client.println("Connection: close");
//     client.println();
//     client.println(jsonData);

//     // Read response
//     String response = "";
//     while (client.connected() || client.available()) {
//       if (client.available()) {
//         response += client.readStringUntil('\n');
//       }
//     }

//     // Serial.println("Server response:");
//     // Serial.println(response);

//     client.stop(); // Close connection
//   }
// }