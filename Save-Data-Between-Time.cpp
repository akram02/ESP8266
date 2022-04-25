#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID "***"
#define WIFI_PASSWORD "***"
#define API_KEY "****-***"
#define DATABASE_URL "***/"
#define USER_EMAIL "*******"
#define USER_PASSWORD "****"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid, databasePath;
FirebaseJson json;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long timestamp;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 5000;
int count = 0;
bool signupOK = false;

void initWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED){
        delay(300);
    }
}

unsigned long getLongTime() {
    timeClient.update();
    unsigned long now = timeClient.getEpochTime();
    return now;
}

String getFormattedTime() {
    timeClient.update();
    String now = timeClient.getFormattedTime();
    return now;
}

void setup(){
  Serial.begin(115200);
  initWiFi();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    Firebase.begin(&config, &auth);

  Serial.println("Getting User UID");
    while (auth.token.uid.empty()) {
        Serial.print(".");
        delay(1000);
    }
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

    databasePath = "/ESPUserTime/" + uid + "/readings";
}

void loop(){
  if ((millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      timeClient.update();
      timestamp = getLongTime();
      json.set("time", getFormattedTime());
      json.set("timestamp", String(timestamp));
      Serial.printf("Current minute is %d\n", timeClient.getMinutes());
      Serial.printf("Current hour is %d\n", timeClient.getHours());
      if (timeClient.getHours()==14 && timeClient.getMinutes()>12 && timeClient.getMinutes()<20) {
          String parentPath = databasePath + "/on/" + String(timestamp);
          Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      }
      else {
          String parentPath = databasePath + "/off/" + String(timestamp);
          Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
      }
  }
}
