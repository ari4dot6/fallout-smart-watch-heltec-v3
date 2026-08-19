#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "a.st1.ntp.br";
const char* NTP_SERVER_3 = "b.st1.ntp.br";

// Sao Paulo / Brasil: UTC-3
const char* TZ_INFO = "BRT3";

bool ntpSynced = false;
uint32_t lastSync = 0;
const uint32_t RESYNC_INTERVAL = 21600000UL; // 6 horas

void drawClock()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 20)) return;

  char hora[9];
  char data[11];
  strftime(hora, sizeof(hora), "%H:%M:%S", &timeinfo);
  strftime(data, sizeof(data), "%d/%m/%Y", &timeinfo);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(3);
  display.setCursor(8, 8);
  display.println(hora);

  display.setTextSize(1);
  display.setCursor(31, 46);
  display.print(data);

  display.display();
}

bool syncNTP()
{
  if (WiFi.status() != WL_CONNECTED) return false;

  configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000))
  {
    Serial.println("NTP: falha");
    return false;
  }

  ntpSynced = true;
  lastSync = millis();

  Serial.printf("NTP OK: %02d:%02d:%02d %02d/%02d/%04d\n",
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec,
                timeinfo.tm_mday,
                timeinfo.tm_mon + 1,
                timeinfo.tm_year + 1900);

  return true;
}

void connectWiFi()
{
  Serial.println("Conectando ao Wi-Fi do Wokwi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    syncNTP();
  }
  else
  {
    Serial.println("Nao foi possivel conectar ao Wi-Fi.");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("Falha no OLED");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println("FALLOUT WATCH");
  display.setCursor(20, 40);
  display.println("NTP STARTING...");
  display.display();

  connectWiFi();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED &&
      (!ntpSynced || millis() - lastSync >= RESYNC_INTERVAL))
  {
    syncNTP();
  }

  if (ntpSynced)
    drawClock();

  delay(500);
}
