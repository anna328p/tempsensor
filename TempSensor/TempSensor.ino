#define API_ENDPOINT "http://1M9SWZ1.internal.castus.tv:4567/"
#define WIFI_SSID "CASTUS [2.4G] [Secure]"
#define WIFI_PSK "Yfunelunsth"

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define USE_SERIAL Serial

#include <Wire.h>
#include "Adafruit_MCP9808.h"

#include <string>

WiFiMulti wifiMulti;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

const char *ca = "-----BEGIN CERTIFICATE-----\n"\
                 "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"\
                 "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"\
                 "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"\
                 "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"\
                 "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"\
                 "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"\
                 "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"\
                 "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"\
                 "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"\
                 "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"\
                 "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"\
                 "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"\
                 "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"\
                 "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"\
                 "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"\
                 "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"\
                 "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"\
                 "rqXRfboQnoZsG4q5WTP468SQvvG5\n"\
                 "-----END CERTIFICATE-----\n";

void setup() {
  USE_SERIAL.begin(115200);

  USE_SERIAL.print("\n\n\n");

  USE_SERIAL.print("Hardware init: wait ");
  for (uint8_t t = 3; t > 0; t--) {
    USE_SERIAL.printf("%d ", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  wifiMulti.addAP(WIFI_SSID, WIFI_PSK);
  Serial.print("\nConnecting");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print(" Connected.\nIP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  USE_SERIAL.print("MCP9808: ");

  // Default address is 0x18, can be changed with A0, A1, A2 pins
  if (!tempsensor.begin(0x18)) {
    USE_SERIAL.println(
      "error!\nCouldn't find MCP9808. Check your connections and verify the address "
      "is correct.");
    while (1)
      ;
  }

  USE_SERIAL.println("found!");

  tempsensor.setResolution(3);  // sets the resolution mode of reading, the
  // modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

  USE_SERIAL.println("Waking MCP9808.... ");

  // wake up MCP9808 - power consumption ~200 uA
  tempsensor.wake();
  delay(200);

  USE_SERIAL.println("Connecting to time server...");
  timeClient.begin();
  timeClient.setTimeOffset(-28800);

  USE_SERIAL.println("Initialization complete.\n\n\n");
}

void loop() {
  HTTPClient http;

  USE_SERIAL.print("Date: ");
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  USE_SERIAL.println(formattedDate);

  USE_SERIAL.print("Temperature: ");
  float c = tempsensor.readTempC();
  String temp_string = String(c);
  USE_SERIAL.printf("%.4f\n", c);

  String json_begin = String("{\"temperature\":");
  String json = String(json_begin + temp_string + ",\"timestamp\":\"" + formattedDate + "\"}");

  USE_SERIAL.print("HTTP: ");
  USE_SERIAL.print("begin; ");
  http.begin(API_ENDPOINT);

  USE_SERIAL.print("POST; ");
  // start connection and send HTTP header
  int httpCode = http.POST(json);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("code: %d; response:\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      USE_SERIAL.println(payload);
    }
  } else {
    USE_SERIAL.printf("failed, error: %s\n",
                      http.errorToString(httpCode).c_str());
  }

  USE_SERIAL.println("");
  http.end();
  delay(200);
}
