#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>

#include "Open_Sans_Hebrew_Bold_80.h"
#include "Open_Sans_Hebrew_Bold_30.h"
#include "Orbitron_Bold_16.h"
#include "Orbitron_Bold_14.h"
#include "Orbitron_Bold_10.h"

#define TFT_DC   D7
#define TFT_RST  D9
#define TFT_BL   D6

Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, -1);
Arduino_GFX *gfx = new Arduino_ST7789(
bus, TFT_RST, 0, true, 240, 240, 0, 0);

#include "menu_start.h"

#define TOUCH0 D0
#define TOUCH1 D1
#define TOUCH2 D2

bool t0=false,t1=false,t2=false;

// ================= WIFI / NTP =================
// Preencha com os dados da sua rede Wi-Fi antes de gravar na Heltec.
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "a.st1.ntp.br";
const char* NTP_SERVER_3 = "b.st1.ntp.br";

// Sao Paulo / Brasil: UTC-3, sem horario de verao atualmente.
const char* TZ_INFO = "BRT3";

bool ntpSynced=false;
uint32_t lastNtpSync=0;
const uint32_t NTP_RESYNC_INTERVAL = 21600000UL; // 6 horas

// ================= TIME =================
int HH=0;
int MIN=0;
int SEC=0;

int DAY=1;
int MON=1;
int YEAR=2026;

// ================= SLEEP =================
bool screenSleeping=false;
uint32_t lastTouchTime=0;
#define SLEEP_NORMAL 10000

// ================= TOUCH =================
void updateTouch()
{
  t0 = touchRead(TOUCH0) > 80000;
  t1 = touchRead(TOUCH1) > 80000;
  t2 = touchRead(TOUCH2) > 80000;

  if(t0 || t1 || t2)
  {
    lastTouchTime=millis();

    if(screenSleeping)
      wakeScreen();
  }
}

// ================= NTP =================
bool syncNTP()
{
  if(WiFi.status() != WL_CONNECTED)
    return false;

  configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo, 10000))
  {
    Serial.println("NTP: falha ao obter hora");
    return false;
  }

  HH = timeinfo.tm_hour;
  MIN = timeinfo.tm_min;
  SEC = timeinfo.tm_sec;
  DAY = timeinfo.tm_mday;
  MON = timeinfo.tm_mon + 1;
  YEAR = timeinfo.tm_year + 1900;

  ntpSynced=true;
  lastNtpSync=millis();

  Serial.printf("NTP OK: %02d:%02d:%02d %02d/%02d/%04d\n", HH, MIN, SEC, DAY, MON, YEAR);
  return true;
}

void connectWiFiAndSync()
{
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<15000)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if(WiFi.status()==WL_CONNECTED)
  {
    Serial.print("Wi-Fi OK: ");
    Serial.println(WiFi.localIP());
    syncNTP();
  }
  else
  {
    Serial.println("Wi-Fi indisponivel. Relogio aguardando sincronizacao NTP.");
  }
}

void updateClock()
{
  // A hora e mantida pelo RTC interno do ESP32 depois da sincronizacao NTP.
  // Fazemos uma nova consulta periodicamente para corrigir eventual deriva.
  if(WiFi.status()==WL_CONNECTED &&
     (!ntpSynced || millis()-lastNtpSync>=NTP_RESYNC_INTERVAL))
  {
    syncNTP();
  }

  static uint32_t displayTimer=0;
  if(millis()-displayTimer<500) return;
  displayTimer=millis();

  struct tm timeinfo;
  if(getLocalTime(&timeinfo, 20))
  {
    HH = timeinfo.tm_hour;
    MIN = timeinfo.tm_min;
    SEC = timeinfo.tm_sec;
    DAY = timeinfo.tm_mday;
    MON = timeinfo.tm_mon + 1;
    YEAR = timeinfo.tm_year + 1900;
  }
}

// ================= SLEEP =================
void sleepScreen()
{
  screenSleeping=true;
  ledcWrite(TFT_BL,0);
}

// ================= WAKE =================
void wakeScreen()
{
  screenSleeping=false;
  ledcWrite(TFT_BL,204);   // 80%

  gfx->fillScreen(0);
  start_draw_static();
  lastTouchTime=millis();
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  Wire.begin(D4,D5);

  pinMode(TFT_BL, OUTPUT);
  ledcAttach(TFT_BL,5000,8);
  ledcWrite(TFT_BL,204);   // 80%

  gfx->begin();
  gfx->fillScreen(0);

  // Tenta sincronizar a hora pela rede.
  connectWiFiAndSync();

  start_draw_static();
  lastTouchTime=millis();
}

// ================= LOOP =================
void loop()
{
  updateTouch();
  updateClock();

  if(!screenSleeping)
    start_update();

  if(!screenSleeping && millis()-lastTouchTime>SLEEP_NORMAL)
    sleepScreen();

  delay(5);
}
