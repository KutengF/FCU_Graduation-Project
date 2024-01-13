#include <ESP8266WiFi.h>
#include "AESLib.h"

AESLib aesLib;

// WiFi credentials
const char* ssid = "Redmi Note 11";
const char* password = "Alex9312";

// Node.js server details
const char* serverIP = "192.168.5.239";
const uint16_t serverPort = 3000;

int Setup = 0;

// AES Encryption Key and Initialization Vector
//byte aes_key[] = { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };
byte aes_key[] = { 0x32, 0x64, 0x33, 0x35, 0x36, 0x31, 0x35, 0x61, 0x32, 0x33, 0x31, 0x61, 0x38, 0x36, 0x39, 0x66 };
byte aes_iv[N_BLOCK] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//byte aes_iv[N_BLOCK] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12 };

String encrypt_impl(char * msg, byte iv[]) {
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen] = {0};
  aesLib.encrypt64((const byte*)msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  return String(encrypted);
}

void clearSerialBuffer() {
    while (Serial.available()) {
        Serial.read();
    }
}

void sendToServer(const String& encryptedData) {
  WiFiClient client;
  
  if (!client.connect(serverIP, serverPort)) {
    Serial.println("Connection to server failed!");
    return;
  }

  // Send POST request
  client.println("POST /data HTTP/1.1");
  client.println("Host: " + String(serverIP));
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(encryptedData.length());
  client.println();
  client.print(encryptedData);

  // Give the server some time to receive and process
  delay(500);

  // Read and print server response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String serverResponse = client.readString();
  Serial.println("Server Response: " + serverResponse);

  client.stop();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  aesLib.set_paddingmode(paddingMode::CMS);
}

void loop() {
  if (Serial.available()) {
    if (Setup == 0)
    {
      for (int x = 0; x < 10; x ++)
        Serial.readStringUntil('\n');
      Setup = 1;
    }
    else
    {
      String readBuffer = Serial.readStringUntil('\n');
      if (readBuffer.startsWith("FromArduino:")) {
        readBuffer = readBuffer.substring(12); // Strip the prefix
        char cleartext[256];
        sprintf(cleartext, "%s", readBuffer.c_str());
        String encrypted = encrypt_impl(cleartext, aes_iv);
        sendToServer(encrypted);
        for (int i = 0; i < 16; i++) {
          aes_iv[i] = 0; // Reset Initialization Vector
        }
      }
      delay(500);
      // Polling the server for updates
      WiFiClient client;
      if (client.connect(serverIP, serverPort)) {
        client.println("GET /getUpdates HTTP/1.1");
        client.println("Host: " + String(serverIP));
        client.println("Connection: close");
        client.println();

        while (client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            break;
          }
        }
        
        String response = client.readString();
        if (!response.isEmpty() && response.startsWith("FromServer:")) {
          response = response.substring(11);  // Strip the prefix
          clearSerialBuffer();
          Serial.println(response);
        }

        client.stop();
      }

      delay(500); // Poll every 0.5 seconds
    }
  }
}
