#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

// Buzzer Pin connect to esp8266 Pin D6

// NTP Client settings
const char* ssid     = "YourSSID"; 
const char* password = "YourPass"; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 2, 60000);  // UTC+1 časová zóna aktualizácia každú minútu

// LED matrix settings
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   D5  // alebo zadajte príslušné piny
#define DATA_PIN  D7  // alebo zadajte príslušné piny
#define CS_PIN    D8  // alebo zadajte príslušné piny

// Buzzer pin
#define BUZZER_PIN D6

MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

bool colonVisible = true;  // Stav dvojbodky (viditeľná/neviditeľná)
unsigned long previousMillis = 0; // Čas posledného preklopenia stavu dvojbodky
const long interval = 1000; // Interval na preklopenie stavu dvojbodky (1 sekunda)

String previousHour = "";  // Premenná na uloženie predchádzajúcej hodiny

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

  // Set up buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  timeClient.update();

  // Získanie formátovaného času (HH:MM:SS)
  String formattedTime = timeClient.getFormattedTime();

  // Extrahovanie hodín a minút
  String hours = formattedTime.substring(0, 2);
  String minutes = formattedTime.substring(3, 5);

  // Kontrola času a preklopenie stavu dvojbodky
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    colonVisible = !colonVisible;  // Zmena stavu dvojbodky

    // Zobrazenie času s alebo bez dvojbodky
    String timeToDisplay = hours + (colonVisible ? ":" : " ") + minutes;

    // Zobrazenie času na LED matrici
    display.displayClear();
    display.displayText(timeToDisplay.c_str(), PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    // Spustenie animácie
    display.displayAnimate();
  }

  // Kontrola zmeny hodiny, okrem časov 23:00 až 05:00
  if (previousHour != hours) {
    previousHour = hours;

    int hourInt = hours.toInt();
    if (hourInt < 23 && hourInt >= 6) {
      // Zabzučanie bzučiaku 2x rýchlo
      for (int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);  // Zapnutie bzučiaku na 200 ms
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);  // Pauza medzi zapnutím
      }
    }
  }

  // Kontrola na čas 5:50
  if (hours == "05" && minutes == "50") {
    for (int i = 0; i < 10; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);  // Zapnutie bzučiaku na 1 sekundu
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);  // Pauza na 1 sekundu
    }
  }
}
