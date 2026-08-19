#pragma once

#include "heart.h"
#include "giftim.h"
#include "gifstrong.h"

extern Arduino_GFX *gfx;

extern int bpm;
extern int HH,MIN,SEC;

#define GREEN 0x07E0
#define BLACK 0x0000


const unsigned char* heartFrames[] =
{
epd_bitmap_tim1,
epd_bitmap_tim2
};

const unsigned char* strongFrames[] =
{
epd_bitmap_icon_strength_3,
epd_bitmap_icon_strength_4,
epd_bitmap_icon_strength_5,
epd_bitmap_icon_strength_6
};

int heartFrame=0;
int strongFrame=0;

int bpmGraph[8]={60,60,60,60,60,60,60,60};
int graphIndex=0;

uint32_t tHeart=0;
uint32_t tStrong=0;
uint32_t tGraph=0;


// ================= STATIC =================
void heart_draw_static()
{
  gfx->drawBitmap(0,0,epd_bitmap_HEART,240,240,GREEN);
}


// ================= HEART =================
void heart_draw_heart()
{
  gfx->fillRect(23,45,60,60,BLACK);

  gfx->drawBitmap(
    23,48,
    heartFrames[heartFrame],
    60,60,
    GREEN
  );

  heartFrame++;
  if(heartFrame>=2) heartFrame=0;
}


// ================= STRONG =================
void heart_draw_strong()
{
  gfx->drawBitmap(
    135,78,
    strongFrames[strongFrame],
    80,120,
    GREEN,BLACK
  );

  strongFrame++;
  if(strongFrame>=4) strongFrame=0;
}


// ================= BPM =================
void heart_draw_bpm()
{
  gfx->fillRect(20,115,63,20,BLACK);

  gfx->setFont(&Orbitron_Bold_16);
  gfx->setCursor(21,130);
  gfx->printf("%03d/m",bpm);
}


// ================= TIME =================
void heart_draw_time()
{
  gfx->fillRect(138,41,86,20,BLACK);

  gfx->setFont(&Orbitron_Bold_16);
  gfx->setCursor(140,55);
  gfx->printf("%02d:%02d:%02d",HH,MIN,SEC);
}


// ================= HP =================
void heart_draw_hp()
{
  gfx->fillRect(140,206,80,20,BLACK);

  gfx->setFont(&Orbitron_Bold_16);
  gfx->setCursor(145,222);
  gfx->printf("HP %03d",bpm);
}


// ================= GRAPH =================
void heart_draw_graph()
{
  gfx->fillRect(12,160,110,50,BLACK);

  for(int i=0;i<6;i++)
  {
    int h = map(bpmGraph[i],40,120,5,55);

    gfx->fillRect(
      14 + i*13,
      210-h,
      8,
      h,
      GREEN
    );
  }
}


// ================= UPDATE =================
void heart_update()
{
  if(millis()-tHeart>400)
  {
    tHeart=millis();
    heart_draw_heart();
    heart_draw_bpm();
    heart_draw_time();
    heart_draw_hp();
  }

  if(millis()-tStrong>400)
  {
    tStrong=millis();
    heart_draw_strong();
  }

  if(millis()-tGraph>900)
  {
    tGraph=millis();

    bpmGraph[graphIndex]=bpm;
    graphIndex++;
    if(graphIndex>=8) graphIndex=0;

    heart_draw_graph();
  }
}