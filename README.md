# ESP32 SuperLoop to FreeRTOS Project

## Aim of the Project
The core goal of this project is to convert a traditional _superloop_ program into a _FreeRTOS_-based design.
Instead of one endless loop handling everything, the program is divided into independent, scheduled tasks.

## What the Project Does
This project connects an ESP32 to a webserver over an (SSL) connection. The device simulates sensor readings and sends them as JSON data to the server at fixed intervals.

## FreeRTOS Tasks and Their Roles:
In a superloop, all operations—WiFi connection, client connection, and data sending—run inside one giant __while(1) loop__. That approach is simple, but it mixes concerns and makes timing and error handling messy. Here, the superloop is broken into separate FreeRTOS tasks:

* __WiFiConnectionTask__ – establishes WiFi connection once.

* __clientConnectionTask__ – ensures secure connection to the server.

* __sendData Task__ – periodically prepares and sends sensor data.

_This separation improves clarity, scheduling, and modularity._

<ins> WiFi Connection Task </ins>

&emsp;&emsp; Connects the ESP32 to WiFi.

&emsp;&emsp; Runs until WiFi is established, then deletes itself.

<ins>Client Connection Task</ins>

&emsp;&emsp;Ensures the ESP32 maintains a secure connection to the webserver.

&emsp;&emsp;Only attempts connection if WiFi is already connected.

<ins>Send Data Task</ins>

&emsp;&emsp;Periodically prepares dummy sensor data using a helper function.

&emsp;&emsp;Sends the data to the server in JSON format over HTTPS.

&emsp;&emsp;Uses "Connection: close" and client.stop() to ensure clean connections.

### Additions Beyond Simple Conversion

### 1. Global Client Instance

A single global WiFiClientSecure client is declared and shared across multiple tasks.

This avoids creating multiple clients that may conflict, while still allowing any task to access the same connection state.

### 2. Mutex for Safe Access

To prevent race conditions when multiple tasks access the global client, a mutex (clientMutex) is created.

Each task must take the mutex before using client, and give it back when done.

This ensures ordered, thread-safe access.
_code logic_
```
if (xSemaphoreTake(clientMutex, portMAX_DELAY)) {
    // Safe access to client
    xSemaphoreGive(clientMutex);
}
```
### 3. Reliable DNS with Google & Cloudflare

By default, ESP32 sometimes struggles with DNS lookups.

The code overrides the DNS configuration to use Google (8.8.8.8) and Cloudflare (1.1.1.1), providing more reliable hostname resolution.

WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, 
            IPAddress(8,8,8,8), IPAddress(1,1,1,1));

### 4. Helper Function for Sensor Data

A helper function modifyData() prepares sensor readings (currently dummy values).

This function is called inside tasks to separate data preparation from network logic.

```
String modifyData(int* temperature, int* humidity) {
// atmospheric readings
}
```
### 5. Task Scheduling with vTaskDelayUntil()

Both the clientConnectionTask and sendDataTask use vTaskDelayUntil() to control execution intervals.

Unlike vTaskDelay(), this keeps the task periodic and precise, avoiding drift over time.

_code logic_
```
vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(8000));
```

### 6. Connection Integrity

Each request uses the `Connection: close`  HTTP header and client.stop() afterward.

This prevents reusing broken SSL channels and ensures the next request starts fresh.


### 7. Dependencies Between Tasks

**Dependencies amongst tasks is as follows:**

clientConnectionTask depends on WiFi:

It will only attempt client.connect() when WiFi.status() == WL_CONNECTED.

sendDataTask depends on the client:

It will only send data if client.connected() == true.

This dependency chain ensures:

* No data is sent if WiFi is down.

* No client reconnect is attempted unless WiFi is active.

* Race conditions are avoided with the mutex.


## Summary

+ This project demonstrates good **FreeRTOS** practices on ESP32:

+ Shared state managed with a **mutex**.

+ Proper **task scheduling** to avoid blocking.

+ Reliable **DNS resolution** for stable server access.

+ **Dependency enforcement** between WiFi, client, and data tasks.
