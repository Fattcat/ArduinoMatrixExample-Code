#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

// NTP Client settings
const char* ssid     = "YourSSID";        // Zadajte názov vašej Wi-Fi siete
const char* password = "YourPass";    // Zadajte heslo k vašej Wi-Fi sieti
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 2, 60000);  // UTC+1 časová zóna, aktualizácia každú minútu

// LED matrix settings
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   D5  // alebo zadajte príslušné piny
#define DATA_PIN  D7  // alebo zadajte príslušné piny
#define CS_PIN    D8  // alebo zadajte príslušné piny

MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  Serial.begin(115200);

  // Initialize LED matrix
  display.begin();
  display.setIntensity(0);  // Nastavte požadovanú intenzitu
  display.displayClear();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize NTPClient
  timeClient.begin();
}

void loop() {
  timeClient.update();

  // Get formatted time
  String formattedTime = timeClient.getFormattedTime();

  // Extract hours and minutes
  String hoursMinutes = formattedTime.substring(0, 5); // "HH:MM"

  // Display time on LED matrix
  display.print(hoursMinutes.c_str());

  delay(1000);
}
