# ESP32 SuperLoop to FreeRTOS Project

This project demonstrates how to use _FreeRTOS tasks_ on an ESP32 to manage WiFi connectivity, secure client connections, and periodic data sending to a server. The design focuses on concurrency safety, reliable DNS resolution, and maintaining communication integrity.

## Key Features
1. Global Client Instance

A single global WiFiClientSecure client is declared and shared across multiple tasks.

This avoids creating multiple clients that may conflict, while still allowing any task to access the same connection state.

## WiFiClientSecure client;

2. Mutex for Safe Access

To prevent race conditions when multiple tasks access the global client, a mutex (clientMutex) is created.

Each task must take the mutex before using client, and give it back when done.

This ensures ordered, thread-safe access.

if (xSemaphoreTake(clientMutex, portMAX_DELAY)) {
    // Safe access to client
    xSemaphoreGive(clientMutex);
}

3. Reliable DNS with Google & Cloudflare

By default, ESP32 sometimes struggles with DNS lookups.

The code overrides the DNS configuration to use Google (8.8.8.8) and Cloudflare (1.1.1.1), providing more reliable hostname resolution.

WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, 
            IPAddress(8,8,8,8), IPAddress(1,1,1,1));

## Helper Function for Sensor Data

A helper function modifyData() prepares sensor readings (currently dummy values).

This function is called inside tasks to separate data preparation from network logic.

String modifyData(int* temperature, int* humidity) {
    *temperature = 30;
    *humidity = 70;
    return "{\"temperature\":" + String(*temperature) + 
           ", \"humidity\":" + String(*humidity) + "}";
}

5. Task Scheduling with vTaskDelayUntil()

Both the clientConnectionTask and sendDataTask use vTaskDelayUntil() to control execution intervals.

Unlike vTaskDelay(), this keeps the task periodic and precise, avoiding drift over time.

vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(8000));

6. Connection Integrity

Each request uses the Connection: close HTTP header and client.stop() afterward.

This prevents reusing broken SSL channels and ensures the next request starts fresh.

client.println("Connection: close");
client.stop();

7. Dependencies Between Tasks

clientConnectionTask depends on WiFi:

It will only attempt client.connect() when WiFi.status() == WL_CONNECTED.

sendDataTask depends on the client:

It will only send data if client.connected() == true.

This dependency chain ensures:

No data is sent if WiFi is down.

No client reconnect is attempted unless WiFi is active.

Race conditions are avoided with the mutex.

if(WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
        client.stop();
        client.connect(server, 443);
    }
}

Task Breakdown

wifiConnectionTask: Connects to WiFi and exits after success.

clientConnectionTask: Ensures client is connected to server (every 5s).

sendDataTask: Prepares JSON data and sends it over HTTPS (every 8s).

Why This Matters

This project demonstrates good FreeRTOS practices on ESP32:

Shared state managed with a mutex.

Proper task scheduling to avoid blocking.

Reliable DNS resolution for stable server access.

Dependency enforcement between WiFi, client, and data tasks.

Clean connection management for SSL/TLS communication.