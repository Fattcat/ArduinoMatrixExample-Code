#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TM1637Display.h>

// NTP Client settings
const char* ssid     = "ssid"; 
const char* password = "password"; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 2, 60000);  // UTC+1 časová zóna aktualizácia každú minútu

// TM1637 settings
#define CLK_PIN  5  // TM1637 CLK pin
#define DIO_PIN  7  // TM1637 DIO pin
TM1637Display display(CLK_PIN, DIO_PIN);

// Buzzer pin
#define BUZZER_PIN 6

bool colonVisible = true;  // Stav dvojbodky (viditeľná/neviditeľná)
unsigned long previousMillis = 0; // Čas posledného preklopenia stavu dvojbodky
const long interval = 1000; // Interval na preklopenie stavu dvojbodky (1 sekunda)

String previousHour = "";  // Premenná na uloženie predchádzajúcej hodiny
bool buzzedAt550 = false;  // Indikátor, či už bzučiak bzučal o 5:50

void setup() {
  Serial.begin(115200);

  // Initialize TM1637 display
  display.setBrightness(0x0f);  // Nastavenie intenzity svietenia

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

    // Zobrazenie času s alebo bez dvojbodky na TM1637 displeji
    int displayTime = (hours.toInt() * 100) + minutes.toInt();
    display.showNumberDecEx(displayTime, colonVisible ? 0x80 : 0x00, true); // Zobraz číslo s dvojbodkou
  }

  // Kontrola zmeny hodiny, okrem časov 23:00 až 05:00
  if (previousHour != hours) {
    previousHour = hours;
    buzzedAt550 = false;  // Reset indikátora po každej hodine

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

  // Kontrola na čas 5:50 a zabezpečenie, že bzučiak zabzučí len raz
  if (hours == "05" && minutes == "50" && !buzzedAt550) {
    buzzedAt550 = true;  // Nastavenie indikátora, aby sa sekvencia nevykonala viackrát
    for (int i = 0; i < 10; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);  // Zapnutie bzučiaku na 1 sekundu
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);  // Pauza na 1 sekundu
    }
  }
}
