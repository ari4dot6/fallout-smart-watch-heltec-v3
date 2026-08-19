#pragma once

#include "start.h"
#include "gifwalk.h"

extern Arduino_GFX *gfx;

extern int HH,MIN,SEC;
extern int DAY,MON,YEAR;

#define GREEN 0x07E0
#define BLACK 0x0000

// ===== GIF =====
const unsigned char* walkFrames[] =
{
  epd_bitmap_icon_luck_7,
  epd_bitmap_icon_luck_8,
  epd_bitmap_icon_luck_9,
  epd_bitmap_icon_luck_10,
  epd_bitmap_icon_luck_11,
  epd_bitmap_icon_luck_12,
  epd_bitmap_icon_luck_13
};

int start_frame=0;

int lastHH=-1;
int lastMM=-1;
int lastDAY=-1;
int lastMON=-1;
int lastYEAR=-1;

bool startForceRedraw=true;
uint32_t start_anim=0;

// ================= STATIC =================
void start_draw_static()
{
  gfx->drawBitmap(0,0,epd_bitmap_start,240,240,GREEN);
  startForceRedraw = true;
}

// ================= HH =================
void start_draw_HH()
{
  gfx->fillRect(8,42,90,60,BLACK);

  gfx->setFont(&Open_Sans_Hebrew_Bold_80);
  gfx->setTextColor(GREEN);
  gfx->setCursor(6,102);
  gfx->printf("%02d",HH);
}

// ================= MM =================
void start_draw_MM()
{
  gfx->fillRect(8,124,90,60,BLACK);

  gfx->setFont(&Open_Sans_Hebrew_Bold_80);
  gfx->setTextColor(GREEN);
  gfx->setCursor(6,182);
  gfx->printf("%02d",MIN);
}

// ================= DATE =================
void start_draw_DATE()
{
  gfx->fillRect(8,208,104,20,BLACK);

  gfx->setFont(&Orbitron_Bold_14);
  gfx->setTextColor(GREEN);
  gfx->setCursor(7,222);
  gfx->printf("%02d.%02d.%04d",DAY,MON,YEAR);
}

// ================= VAULT =================
void start_draw_vault()
{
  gfx->drawBitmap(
    145,
    78,
    walkFrames[start_frame],
    80,
    120,
    GREEN,
    BLACK
  );

  start_frame++;
  if(start_frame>=7) start_frame=0;
}

// ================= UPDATE =================
void start_update()
{
  if(startForceRedraw)
  {
    start_draw_HH();
    start_draw_MM();
    start_draw_DATE();

    lastHH=HH;
    lastMM=MIN;
    lastDAY=DAY;
    lastMON=MON;
    lastYEAR=YEAR;

    startForceRedraw=false;
  }

  // Animation do Vault Boy.
  if(millis()-start_anim>140)
  {
    start_anim=millis();
    start_draw_vault();
  }

  if(HH!=lastHH)
  {
    lastHH=HH;
    start_draw_HH();
  }

  if(MIN!=lastMM)
  {
    lastMM=MIN;
    start_draw_MM();
  }

  if(DAY!=lastDAY || MON!=lastMON || YEAR!=lastYEAR)
  {
    lastDAY=DAY;
    lastMON=MON;
    lastYEAR=YEAR;
    start_draw_DATE();
  }
}
