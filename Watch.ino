#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

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
#include "menu_heart.h"
#include "menu_data.h"

#define TOUCH0 D0
#define TOUCH1 D1
#define TOUCH2 D2

bool t0=false,t1=false,t2=false;
bool lastT0=false,lastT1=false,lastT2=false;


// ================= TIME =================
int HH=16;
int MIN=30;
int SEC=0;

int DAY=25;
int MON=4;
int YEAR=26;


// ================= BPM SPO2 =================
int bpm=100;
int spo2=100;


// ================= MENU =================
enum MenuState
{
MENU_START,
MENU_HEART,
MENU_DATA
};

int currentMenu = MENU_START;


// ================= SENSOR =================
MAX30105 particleSensor;

long lastBeat = 0;

float irAvg=0;
float redAvg=0;

int bpmSamples[3];
int spo2Samples[3];

int bpmIndex=0;
int spo2Index=0;

uint32_t nextMeasure=0;

enum MeasureState
{
WAIT_FINGER,
MEASURE_BPM,
MEASURE_SPO2
};

MeasureState measureState=WAIT_FINGER;


// ================= SLEEP =================
bool screenSleeping=false;
bool pendingSleep=false;

uint32_t lastTouchTime=0;

#define SLEEP_NORMAL 10000
#define SLEEP_HEART 180000


// ================= TOUCH =================
void updateTouch()
{
t0 = touchRead(TOUCH0) > 80000;
t1 = touchRead(TOUCH1) > 80000;
t2 = touchRead(TOUCH2) > 80000;

if(t0||t1||t2)
{
lastTouchTime=millis();

if(screenSleeping && t1)
wakeScreen();
}
}


// ================= CLOCK =================
uint32_t clockTimer=0;

void updateClock()
{
if(millis()-clockTimer<1000) return;
clockTimer=millis();

SEC++;

if(SEC>=60){SEC=0;MIN++;}
if(MIN>=60){MIN=0;HH++;}
if(HH>=24){HH=0;DAY++;}

int maxDay=data_daysInMonth(MON,YEAR);

if(DAY>maxDay)
{
DAY=1;
MON++;

if(MON>12)
{
MON=1;
YEAR++;
}
}
}


// ================= SENSOR =================
void sensorUpdate()
{
long ir = particleSensor.getIR();
long red = particleSensor.getRed();

if(measureState==WAIT_FINGER)
{
if(ir>7000)
{
measureState=MEASURE_BPM;
bpmIndex=0;
spo2Index=0;
}
}

else if(measureState==MEASURE_BPM)
{
if (checkForBeat(ir))
{
long delta = millis() - lastBeat;
lastBeat = millis();

float beatsPerMinute = 60 / (delta / 1000.0);

if (beatsPerMinute < 180 && beatsPerMinute > 40)
{
bpmSamples[bpmIndex++] = beatsPerMinute;

if(bpmIndex>=3)
{
int avg=0;
for(int i=0;i<3;i++) avg+=bpmSamples[i];

bpm = avg/3;

measureState=MEASURE_SPO2;
}
}
}
}

else if(measureState==MEASURE_SPO2)
{
irAvg = irAvg*0.95 + ir*0.05;
redAvg = redAvg*0.95 + red*0.05;

float acIR = ir - irAvg;
float acRed = red - redAvg;

float ratio = (acRed/redAvg)/(acIR/irAvg);

int val = 110 - 25 * ratio;

if(val>100) val=100;
if(val<80) val=80;

static uint32_t t=0;

if(millis()-t>800)
{
t=millis();

spo2Samples[spo2Index++] = val;

if(spo2Index>=3)
{
int avg=0;
for(int i=0;i<3;i++) avg+=spo2Samples[i];

spo2 = avg/3;

measureState=WAIT_FINGER;

nextMeasure = millis()+180000;
}
}
}
}


// ================= SLEEP =================
void sleepScreen()
{
screenSleeping=true;

particleSensor.shutDown();

// tắt màn
ledcWrite(TFT_BL,0);
}


// ================= WAKE =================
void wakeScreen()
{
screenSleeping=false;

particleSensor.wakeUp();
particleSensor.setup();

// sáng 80%
ledcWrite(TFT_BL,204);

gfx->fillScreen(0);

if(currentMenu==MENU_START) start_draw_static();
if(currentMenu==MENU_HEART) heart_draw_static();
if(currentMenu==MENU_DATA) data_draw_static();

lastTouchTime=millis();
}


// ================= SETUP =================
void setup()
{
Wire.begin(D4,D5);

particleSensor.begin(Wire, I2C_SPEED_FAST);
particleSensor.setup();

particleSensor.setPulseAmplitudeRed(0x0A);
particleSensor.setPulseAmplitudeIR(0x0A);

// PWM backlight
pinMode(TFT_BL, OUTPUT);
ledcAttach(TFT_BL,5000,8);
ledcWrite(TFT_BL,204);   // 80%

gfx->begin();
gfx->fillScreen(0);

start_draw_static();

lastTouchTime=millis();
}


// ================= LOOP =================
void loop()
{
updateTouch();
updateClock();

if(millis()>nextMeasure && !screenSleeping)
sensorUpdate();

uint32_t sleepTime =
(currentMenu==MENU_HEART) ? SLEEP_HEART : SLEEP_NORMAL;

//sleep
if(!screenSleeping && millis()-lastTouchTime>sleepTime)
pendingSleep=true;

// sleep
if(pendingSleep && measureState==WAIT_FINGER)
{
sleepScreen();
pendingSleep=false;
}

if(!screenSleeping)
{
switch(currentMenu)
{
case MENU_START: start_update(); break;
case MENU_HEART: heart_update(); break;
case MENU_DATA: data_update(); break;
}
}

if(!editMode && !screenSleeping)
{
if(t0 && !lastT0)
{
if(currentMenu==MENU_HEART)
{
currentMenu=MENU_START;
gfx->fillScreen(0);
start_draw_static();
}
else if(currentMenu==MENU_DATA)
{
currentMenu=MENU_HEART;
gfx->fillScreen(0);
heart_draw_static();
}
}

if(t2 && !lastT2)
{
if(currentMenu==MENU_START)
{
currentMenu=MENU_HEART;
gfx->fillScreen(0);
heart_draw_static();
}
else if(currentMenu==MENU_HEART)
{
currentMenu=MENU_DATA;
gfx->fillScreen(0);
data_draw_static();
}
}
}

lastT0=t0;
lastT1=t1;
lastT2=t2;
}