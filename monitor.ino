#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Adafruit_LiquidCrystal.h"

// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "coronavirus-19-api.herokuapp.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "08 3B 71 72 02 43 6E CA ED 42 86 93 BA 7E DF 81 C4 BC 62 30";

// called once on startup
void setup() {
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);

  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial) continue;

  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Let's request the data, but no more than once every 5 minutes.
  Serial.println("Requesting data!");
  getData();
  // and wait at least 5 minutes before doing it again
  delay(300000);
}

void getData() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/countries/usa";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: DEFCON\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String json = client.readStringUntil('\n');
    if (json == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String json = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(json);
  Serial.println("==========");

  // Allocate JsonBuffer
  // Used arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(8) + 90;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  deserializeJson(doc, json);

  // Extract raw value
  int active = doc["active"]; // 8742
  int today = doc["todayCases"]; // 274

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  lcd.print("COVID-19 US CASES");
  lcd.setCursor(0, 1);
  lcd.print("--------------------");
  lcd.setCursor(0, 2);
  lcd.print("ACTIVE: " + String(active));
  lcd.setCursor(0, 3);
  lcd.print("NEW TODAY: " + String(today));

  lcd.setBacklight(HIGH);
}
