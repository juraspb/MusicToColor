#include <Adafruit_NeoPixel.h>
 
#define ledPin 13       // светодиод на плате arduino
#define stripPin 2      // выход управления светодиодной лентой
#define stripLed 20    // количество светодиодов в ленте
#define bandPass 10     // число полос ЦМУ (используемых в программах)
#define LedtoColor stripLed/bandPass
#define fallspeed 15

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLed, stripPin, NEO_GRB + NEO_KHZ800);
  
typedef union{
    struct { uint8_t b,g,r,w; };
    uint32_t dw;
} TColor;

typedef union{
    struct { uint8_t b0,b1; };
    uint16_t w;
} TWord;

typedef union{
    struct { uint8_t b0,b1,b2,b3; };
    uint32_t dw;
} TDWord;

const uint32_t PROGMEM
  colorTab[]={
      0xFF0000,0xFF1100,0xFF2200,0xFF3300,0xFF4400,0xFF5500,0xFF6600,0xFF7700,0xFF8800,0xFF9900,0xFFAA00,0xFFBB00,0xFFCC00,0xFFDD00,0xFFEE00,0xFFFF00,  //красный - жёлтый
      0xFFFF00,0xEEFF00,0xDDFF00,0xCCFF00,0xBBFF00,0xAAFF00,0x99FF00,0x88FF00,0x77FF00,0x66FF00,0x55FF00,0x44FF00,0x33FF00,0x22FF00,0x11FF00,0x00FF00,  //жёлтый - зелёный
      0x00FF00,0x00FF11,0x00FF22,0x00FF33,0x00FF44,0x00FF55,0x00FF66,0x00FF77,0x00FF88,0x00FF99,0x00FFAA,0x00FFBB,0x00FFCC,0x00FFDD,0x00FFEE,0x00FFFF,  //зелёный - циан (голубой)
      0x00FFFF,0x00EEFF,0x00DDFF,0x00CCFF,0x00BBFF,0x00AAFF,0x0099FF,0x0088FF,0x0077FF,0x0066FF,0x0055FF,0x0044FF,0x0033FF,0x0022FF,0x0011FF,0x0000FF,  //голубой - синий
      0x0000FF,0x1100FF,0x2200FF,0x3300FF,0x4400FF,0x5500FF,0x6600FF,0x7700FF,0x8800FF,0x9900FF,0xAA00FF,0xBB00FF,0xCC00FF,0xDD00FF,0xEE00FF,0xFF00FF,  //синий - пурпур (маджента)
      0xFF00FF,0xFF00EE,0xFF00DD,0xFF00CC,0xFF00BB,0xFF00AA,0xFF0099,0xFF0088,0xFF0077,0xFF0066,0xFF0055,0xFF0044,0xFF0033,0xFF0022,0xFF0011,0xFF0000,  //маджента - красный
      0xFFFFFF,0xEFEFEF,0xDFDFDF,0xCFCFCF,0xBFBFBF,0xAFAFAF,0x9F9F9F,0x8F8F8F,0x7F7F7F,0x6F6F6F,0x5F5F5F,0x4F4F4F,0x3F3F3F,0x2F2F2F,0x1F1F1F,0x0F0F0F,0X000000};

const uint8_t PROGMEM rotateInterval[] = {224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224}; 

const TDWord PROGMEM progColor[] = {
  { 0,16,64,80},   // Цвета первой субпрограммы 1,2,3,4 цвета соответственно
  {16,32,64,48},   // Цвета второй субпрограммы
  {32,64,16,48},
  {48, 0,80,16},
  {64,16,32,80},
  {80,16, 0,48},
  {96,64,48,16},
  {96, 0,32,64}   // Цвета восьмой субпрограммы
  };
/*
0  - красный
16 - желтый
32 - зеленый
48 - аква
64 - синий
80 - фиолетовый
*/

TColor clN[4] = {{0,0,255,0},{0,255,0,0},{0,0,255,0},{0,255,255,0}}; //0xFF0000,0x00FF00,0x0000FF,0xFFFF00
uint8_t stepN[4]  = {0,0,0,0};
uint8_t waitN[4]  = {3,3,4,4};
uint8_t waitNc[4] = {3,3,4,4};
uint8_t magn[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
uint8_t threshold[20] = {192,192,192,192,192,160,160,160,128,128,128,112,112,112,96,96,96,80,80,80};  
uint8_t paramTabl[32] = {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20};
uint8_t olds[bandPass];  
int16_t diff[8][bandPass]; 
uint8_t diffCount = 0;
uint8_t inCounter = 0;
boolean inComplete = false;
uint8_t prog = 2;
uint8_t param = 100;
uint8_t subprog = 7;
uint8_t subcolor = 0;
uint8_t progStep = 0;
uint8_t progSubStep = 0;
uint8_t progDir = 0;
uint8_t brightness = 255;
uint8_t control = 1;
uint8_t rotateStep = 0;
uint8_t dist = 0;
uint32_t clBlack; 
uint32_t clWhite; 
uint8_t bpData[20];
uint8_t rndNumber1 = 0;
uint8_t rndNumber2 = 0;
uint8_t nPhase = 0;

uint8_t inStr[32];            
uint8_t readData[32];         

void setup() {
  // initialize serial:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  clWhite=strip.Color(255, 255, 255);
  clBlack=strip.Color(0, 0, 0);
  Serial.begin(115200);
  // reserve 32 bytes for the inputString:
  pinMode(ledPin, OUTPUT);
}

TColor clOne(uint8_t spr) {
  TColor cl;

  cl.dw=pgm_read_dword(&colorTab[spr * 16]);
  return cl;
}

void sub12() {

  sub12_1(clN[0]);
  strip.show();
  if (++progStep>38) {
    progStep=0;
    progDir=!progDir;
    if ((subprog==7)and(progDir!=0)) {
       clN[0]=clOne(subcolor);
       if (++subcolor>6) subcolor=0;
    }
  }
}

void sub12_1(TColor color) {
  TColor cl;
  TWord akk;
  uint8_t d;
  int16_t r;

  for(uint8_t i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progDir==0) d=progStep*6;
             else d=(38-progStep)*6;
  r=color.r-d;
  if (r<0) r=0;
  akk.w=r*r; 
  cl.r=akk.b1;
  r=color.g-d;
  if (r<0) r=0;
  akk.w=r*r; 
  cl.g=akk.b1;
  r=color.b-d;
  if (r<0) r=0;
  akk.w=r*r; 
  cl.b=akk.b1;
  cl=mulsBrightness(cl);
  strip.setPixelColor(0, cl.dw);
}

void sub11() {
  TColor clF,clD;
  uint8_t i;

  clF=mulsBrightness(clN[0]);
  clD=mulsBrightness(clN[1]);
  for(i=0; i<progStep*4; i++) strip.setPixelColor(i,clD.dw);
  for(i=stripLed-progStep*4; i<stripLed; i++) strip.setPixelColor(i,clD.dw);
  for(i=progStep*4; i<stripLed-progStep*4; i++) strip.setPixelColor(i,clF.dw);

  for(i=progSubStep; i<progSubStep+4; i++) strip.setPixelColor(i,clD.dw);
  for(i=stripLed-progSubStep-4; i<stripLed-progSubStep; i++) strip.setPixelColor(i,clD.dw);
  strip.show();
  progSubStep--;
  if (progSubStep==progStep*4) {
    progStep++;
    if (progStep>stripLed/8-1) {
      progStep=0;
      clF=clN[0];
      clN[0]=clN[1];
      if (subprog==7) {
        clN[1]=clOne(subcolor++);
        if (subcolor>6) subcolor=0;
      } else clN[1]=clF;
    }
    progSubStep=stripLed-1; 
  }
}
void sub10() {
  TColor clF,clD;
  uint8_t i;

  clF=mulsBrightness(clN[0]);
  clD=mulsBrightness(clN[1]);
  for(i=0; i<progStep*4; i++) strip.setPixelColor(i,clD.dw);
  for(i=progStep*4; i<stripLed; i++) strip.setPixelColor(i,clF.dw);
  for(i=progSubStep; i<progSubStep+4; i++) strip.setPixelColor(i,clD.dw);
  strip.show();
  progSubStep--;
  if (progSubStep==progStep*4) {
    progStep++;
    if (progStep>stripLed/4-1) {
      progStep=0;
      clF=clN[0];
      clN[0]=clN[1];
      if (subprog==7) {
        clN[1]=clOne(subcolor++);
        if (subcolor>6) subcolor=0;
      } else clN[1]=clF;
    }
    progSubStep=stripLed-1; 
  }
}

void sub9() {
   TColor cl;

   for(uint8_t i=0; i<4; i++) {
     waitN[i]--;  
     if (waitN[i]==0){
       waitN[i]=waitNc[i];
       stepN[i]++;
       if (stepN[i]>stripLed-1) stepN[i]=0;
       cl=mulsBrightness(clN[i]);
       if (i%2==0) strip.setPixelColor(stepN[i], cl.dw);
              else strip.setPixelColor(stripLed-1-stepN[i], cl.dw);
     }
   }
   strip.show();
   progStep++;
}

void sub8() {
  TColor cl;
  
  for(uint8_t i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>15) cl=mulsBrightness(clN[0]);
              else cl=mulsBrightness(clN[1]);
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
  if (++progStep>19) progStep=0;
}

void sub7() {
  TColor cl;
  
  for(uint8_t i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>17) cl=mulsBrightness(clN[0]);
              else cl=mulsBrightness(clN[1]);
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
  if (++progStep>19) progStep=0;
}

void sub6() {
  TColor cl;

  if (progDir==0) cl=mulsBrightness(clN[0]);
             else cl=mulsBrightness(clN[1]);
  strip.setPixelColor(progStep ,cl.dw);            
  strip.show();
  if (++progStep>stripLed-1) {
    progStep=0;
    progDir=!progDir;
  }
}

void sub5() {
  TColor cl;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progDir==0) cl=mulsBrightness(clN[0]);
             else cl=mulsBrightness(clN[1]);
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
  if (++progStep>9) {
    progStep=0;
    progDir=!progDir;
  }
}

void sub4() {

  sub4_1(clN[0]);
  strip.show();
  if (++progStep>19) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
}

void sub4_1(TColor color) {
  TColor cl;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<10) cl.dw = strip.Color(color.r >> progStep, color.g >> progStep, color.b >> progStep);
              else cl.dw = strip.Color(color.r >> 19-progStep, color.g >> 19-progStep, color.b >> 19-progStep); 
  cl=mulsBrightness(cl);
  strip.setPixelColor(0, cl.dw);
}

void sub3() {

  sub3_1(clN[0]);
  strip.show();
  if (++progStep>23) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
}

void sub3_1(TColor color) {
  TColor cl;
  TWord akk;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<10) {
    if (progStep<2) {
      if (progStep==0) cl.dw = strip.Color(color.r >> 2, color.g >> 2, color.b >> 2);
                  else cl.dw = strip.Color(color.r, color.g, color.b);
    } else cl.dw = strip.Color(color.r >> (progStep-2), color.g >> (progStep-2), color.b >> (progStep-2));
  } else cl.dw = clBlack;
  cl=mulsBrightness(cl);
  strip.setPixelColor(0, cl.dw);
}

void sub2() {

  sub2_1(clN[0]);
  strip.show();
  if (++progStep>7) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
}

void sub2_1(TColor cl) {
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<4) {
    cl=mulsBrightness(cl);
  } else cl.dw=clBlack; 
  strip.setPixelColor(0, cl.dw);
}

void sub1() {

  sub1_1(clN[0]);
  strip.show();
  if (++progStep>16) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
}

void sub1_1(TColor cl) {
  TColor clC;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<8) {
     if (progStep<2) clC.dw=clWhite;
                else clC=cl;
  }
     else {
      if (progStep<10) clC.dw=clWhite;
                  else clC.dw=clBlack; 
     }
  cl=mulsBrightness(clC);
  strip.setPixelColor(0, cl.dw);
}
   
void sub00() {
  uint8_t i;
  TColor cl;
  TWord akk;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl=mulsBrightness(cl);
  } else cl.dw = clBlack;
  strip.setPixelColor(0, cl.dw);
  strip.show();
  if (++progStep>subprog) progStep=0;
}

void sub01() {
  uint8_t i;
  TColor cl;

  for(i=stripLed/2-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  for(i=stripLed/2; i<stripLed; i++) strip.setPixelColor(i,strip.getPixelColor(i+1));
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl=mulsBrightness(cl);
  } else cl.dw = clBlack;
  strip.setPixelColor(0, cl.dw);
  strip.setPixelColor(stripLed-1, cl.dw);
  strip.show();
  if (++progStep>subprog-3) progStep=0;
}

void sub06() {
  uint8_t i;
  TColor cl;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep==0) {
    if (++subcolor>6) subcolor=0;
    cl=mulsBrightness(clOne(subcolor));
    strip.setPixelColor(0, cl.dw);
  }
  strip.show();
  if (++progStep>8) progStep=0;
}

void sub07() {
  uint8_t i;
  TColor cl;

  for(i=stripLed/2-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  for(i=stripLed/2; i<stripLed-1; i++) strip.setPixelColor(i,strip.getPixelColor(i+1));
  if (progStep==0) {
    if (++subcolor>6) subcolor=0;
    cl=mulsBrightness(clOne(subcolor));
    strip.setPixelColor(0, cl.dw);
    strip.setPixelColor(stripLed-1, cl.dw);
  }
  strip.show();
  if (++progStep>8) progStep=0;
}

uint32_t bandMagn(uint8_t n) {
//  TWord akk;
  TColor cl;
  uint8_t gain;
  
//  akk.w=readData[n]*brightness;
//  gain=akk.b1;
  gain=(readData[n]*brightness)>>8;
  if (magn[n]<fallspeed) magn[n] = 0;
                    else magn[n] -= fallspeed; 
  if (magn[n]<gain) magn[n]=gain;
               else gain=magn[n]; 
  cl.dw = pgm_read_dword(&colorTab[96*n/bandPass]);
  cl = mulsGain(cl,gain);
  return strip.Color(cl.r, cl.g, cl.b);
}

void zRainbow() {
  uint32_t cl;
  uint8_t i,k,n;

  for(i=0; i<bandPass; i++) {
      cl = bandMagn(i);
      n=i*LedtoColor;
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(n+k, cl);
  }
  strip.show();
}

void zRainbow10() {
  TColor cl;
  uint8_t gain;
  uint8_t n;
  
  for(uint8_t i=0; i<bandPass/2; i++) {
      gain=((readData[i*2]+readData[i*2+1])*brightness)>>8;
      cl.dw = pgm_read_dword(&colorTab[96*i/(bandPass/2)]);
      cl = mulsGain(cl,gain);
      n=i*stripLed/(bandPass/2);
      for(uint8_t k=0; k<stripLed/6; k++) strip.setPixelColor(n+k, cl.dw);
  }
  strip.show();
}

void zMIX() {
  uint32_t cl;
  uint8_t i,k;

  for(i=0; i<bandPass; i++) {
      cl = bandMagn(i);
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(i+k*bandPass, cl);
  }
  strip.show();
}

void zIBeam() {
  uint32_t cl;
  uint8_t n,i,k;

  for(i=0; i<bandPass; i++) {
      cl = bandMagn(i);
      n=i*LedtoColor/2;
      for(k=0; k<LedtoColor/2; k++) {
        strip.setPixelColor(n+k, cl);
        strip.setPixelColor(stripLed-(n+k)-1, cl);
      }
  }
  strip.show();
}

void zMidland() {
  uint32_t cl;
  uint8_t n,i,k;

  for(i=0; i<bandPass; i++) {
      cl = bandMagn(i);
      n=i*LedtoColor/2;
      for(k=0; k<LedtoColor/2; k++) {
        strip.setPixelColor(stripLed/2+n+k, cl);
        strip.setPixelColor(stripLed/2-(n+k)-1, cl);
      }
  }
  strip.show();
}

void fromCenterShift() {
  uint8_t i;
  uint32_t c;
  
  for(i=0; i<stripLed/2; i++) { c=strip.getPixelColor(i+1); strip.setPixelColor(i,c); }
  for(i=stripLed-1; i>stripLed/2; i--) { c=strip.getPixelColor(i-1); strip.setPixelColor(i,c); }
}

void toCenterShift() {
  uint8_t i;
  uint32_t c;
  
  for(i=stripLed/2; i<stripLed-1; i++) { c=strip.getPixelColor(i+1); strip.setPixelColor(i,c); }
  for(i=stripLed/2-1; i>0; i--) { c=strip.getPixelColor(i-1); strip.setPixelColor(i,c); }
}

void zMagicFrom() {
  TColor cl;
  uint32_t clW;
  uint8_t i,k,j;

  fromCenterShift();
  k=0;
  j=readData[0];
  for(i=1; i<bandPass; i++) {
      if (j<readData[i]) {
        j=readData[i];
        k=i;
      }
  }
  if (readData[k]<12) clW = strip.Color(32, 32, 32);
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    cl=mulsBrightness(cl);
    clW = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(stripLed/2, clW);
  strip.setPixelColor(stripLed/2-1, clW);
  strip.show();
}

void zMagicTo() {
  TColor cl;
  uint32_t clW;
  uint8_t i,k,j;

  toCenterShift();
  k=0;
  j=readData[0];
  for(i=1; i<bandPass; i++) {
      if (j<readData[i]) {
        j=readData[i];
        k=i;
      }
  }
  if (readData[k]<12) clW = strip.Color(32, 32, 32);
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    cl=mulsBrightness(cl);
    clW = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(0, clW);
  strip.setPixelColor(stripLed-1, clW);
  strip.show();
}

void zFeeryFrom() {
  TColor cl;
  uint8_t i,k,j;
  int16_t gain,m;

  fromCenterShift();
  m=0;
  for(i=0; i<bandPass; i++) {
    diff[diffCount][i]=readData[i]-olds[i];
    olds[i]=readData[i];
    gain=0;
    for(k=0; k<8; k++) gain += diff[k][i];
    if (gain<0) gain=-gain;
    if (m<gain) {
      m=gain;
      j=i;
    }
  }  
  
  if (m<6) cl.dw = strip.Color(32, 32, 32);
  else {
   if (m<128) cl.dw = clBlack;
    else {
      cl.dw = pgm_read_dword(&colorTab[96*j/bandPass]);
      cl=mulsBrightness(cl);
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
    }
  }
  strip.setPixelColor(stripLed/2, cl.dw);
  strip.setPixelColor(stripLed/2-1, cl.dw);
  strip.show();
  diffCount++;
  diffCount &= 7;
}

void zFeeryTo() {
  TColor cl;
  uint8_t i,k,j;
  int16_t gain,m;

  toCenterShift();
  m=0;
  for(i=0; i<bandPass; i++) {
    diff[diffCount][i]=readData[i]-olds[i];
    olds[i]=readData[i];
    gain=0;
    for(k=0; k<8; k++) gain += diff[k][i];
    if (gain<0) gain=-gain;
    if (m<gain) {
      m=gain;
      j=i;
    }
  }  
  
  if (m<6) cl.dw = strip.Color(32, 32, 32);
  else {
   if (m<128) cl.dw = clBlack;
    else {
      cl.dw = pgm_read_dword(&colorTab[96*j/bandPass]);
      cl=mulsBrightness(cl);
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
    }
  }
  strip.setPixelColor(0, cl.dw);
  strip.setPixelColor(stripLed-1, cl.dw);
  strip.show();
  diffCount++;
  diffCount &= 7;
}

void zAllureFrom() {
  TColor cl;
  uint32_t clW;
  uint8_t i,k,m;

  fromCenterShift();
  m=0;
  for(i=0; i<bandPass; i++) {
    if (readData[i]-magn[i]>0) {
      if (m<readData[i]) {
        m=readData[i];
        k=i;
      }
    }
    magn[i]=readData[i];
  }
  if (m<6) clW = strip.Color(32, 32, 32);
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    cl=mulsBrightness(cl);
    clW = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(stripLed/2, clW);
  strip.setPixelColor(stripLed/2-1, clW);
  strip.show();
}

void zAllureTo() {
  TColor cl;
  uint32_t clW;
  uint8_t i,k,m;

  toCenterShift();
  m=0;
  for(i=0; i<bandPass; i++) {
    if (readData[i]-magn[i]>0) {
      if (m<readData[i]) {
        m=readData[i];
        k=i;
      }
    }
    magn[i]=readData[i];
  }
  if (readData[k]<6) clW = strip.Color(32, 32, 32);
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    cl=mulsBrightness(cl);
    clW = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(0, clW);
  strip.setPixelColor(stripLed-1, clW);
  strip.show();
}

void skazka() {
  TColor cl;
  TWord akk;
  uint8_t i;
  uint16_t thsumm = 0;
  uint8_t th;

  for (i=0;i<bandPass;i++) thsumm += readData[i];
  thsumm /= bandPass;

  for (i=0;i<bandPass;i++) {
    th= (thsumm * threshold[i])>>7;
    if (readData[i]>th) magn[i]=250;
      else {
       if (magn[i]>9) magn[i]-=10;
      }
  }   
  for (i=0;i<bandPass;i++) {
    akk.w=magn[i]*brightness;
    uint8_t gain=akk.b1;
    cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
    cl=mulsGain(cl,gain);
    cl.dw=strip.Color(cl.r,cl.g,cl.b);
    uint8_t n=i*LedtoColor/2;
    for(uint8_t k=0; k<LedtoColor/2; k++) {
      strip.setPixelColor(stripLed/2+n+k, cl.dw);
      strip.setPixelColor(stripLed/2-(n+k)-1, cl.dw);
    }
  }   
  strip.show();
}

void zLevel() {
  TColor cl;
  TWord akk;
  uint8_t i,k,j;
  uint8_t m=0;
  uint16_t n;

  for(i=0; i<bandPass; i++) {
    if (magn[i]<fallspeed*2) magn[i] = 0;
                        else magn[i] -= fallspeed; 
    if (magn[i]<readData[i]) magn[i]=readData[i];
    if (m<magn[i]) m=magn[i];
  }
  if (m>16) {
    k=0;
    for(i=0; i<bandPass; i++) {
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      akk.w = cl.r * brightness; cl.r = akk.b1;
      akk.w = cl.g * brightness; cl.g = akk.b1;
      akk.w = cl.b * brightness; cl.b = akk.b1;
      akk.w=LedtoColor*magn[i];
      n=akk.b1;
      for(j=0; j<n; j++) {
        strip.setPixelColor(stripLed/2+k, cl.dw);
        strip.setPixelColor(stripLed/2-k-1, cl.dw);
        k++;
      }
    }
    if (k<stripLed/2) {
      for(j=k; j<stripLed/2; j++) {
        strip.setPixelColor(stripLed/2+k, clBlack);
        strip.setPixelColor(stripLed/2-k-1, clBlack);
        k++;
      }
    }
  }
  strip.show();
}

uint32_t bandMagn2(uint8_t n) {
//  TWord akk;
  TColor cl;
  uint16_t gain;
  
  cl.dw = pgm_read_dword(&colorTab[96*n/(bandPass/2)]);
  gain=((readData[2*n]+readData[2*n+1])*brightness)>>8;
  cl = mulsGain(cl,gain);
  return strip.Color(cl.r, cl.g, cl.b);
}

void zIBeam2() {
  uint32_t cl;
  uint8_t n,i,k;

  for(i=0; i<bandPass/2; i++) {
      cl = bandMagn2(i);
      n=i*LedtoColor;
      for(k=0; k<LedtoColor; k++) {
        strip.setPixelColor(n+k, cl);
        strip.setPixelColor(stripLed-(n+k)-1, cl);
      }
  }
  strip.show();
}

void zMidland2() {
  uint32_t cl;
  uint8_t n,i,k;

  for(i=0; i<bandPass/2; i++) {
      cl = bandMagn2(i);
      n=i*LedtoColor;
      for(k=0; k<LedtoColor; k++) {
        strip.setPixelColor(stripLed/2+n+k, cl);
        strip.setPixelColor(stripLed/2-(n+k)-1, cl);
      }
  }
  strip.show();
}

void zCharm() {
  TColor cl;
  uint8_t i,k,j;
  uint16_t summ = 0;
  uint16_t n;

  for(i=0; i<bandPass; i++) {
    if (magn[i]<fallspeed) magn[i] = 0;  
                      else magn[i] -= fallspeed;
    if (magn[i]<readData[i]) magn[i]=readData[i];
    summ += magn[i];
  }
  k=0;
  if (summ>16) {
    for(i=0; i<bandPass; i++) {
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      cl=mulsBrightness(cl);
      n=(stripLed*magn[i]/summ+1)/2;
      for(j=0; j<n; j++) {
        strip.setPixelColor(stripLed/2+k, cl.dw);
        strip.setPixelColor(stripLed/2-k-1, cl.dw);
        k++;
      }
    }
    if (k<stripLed/2) {
      for(j=k; j<stripLed/2; j++) {
        strip.setPixelColor(stripLed/2+k, clBlack);
        strip.setPixelColor(stripLed/2-k-1, clBlack);
        k++;
      }
    }
  }
  strip.show();
}

void stroboscope(uint8_t wait) {
   TColor cl;
   uint8_t i,j;
   
  cl.dw = strip.Color(255, 255, 255);
  for(i=0; i<stripLed-1; i++) strip.setPixelColor(i, cl.dw);
  strip.show();
  delay(wait);
  cl.dw = strip.Color(0, 0, 0);
  for(i=0; i<stripLed-1; i++) strip.setPixelColor(i, cl.dw);
  strip.show();
}

void flashRandom() {
  TColor cl;
  
  rndNumber1 = random(stripLed-1);
  strip.setPixelColor(rndNumber1, clWhite);
  rndNumber2 = random(stripLed-1);
  strip.setPixelColor(rndNumber2, clWhite);
  strip.show();
  delay(10);
  strip.setPixelColor(rndNumber1, clBlack);
  strip.setPixelColor(rndNumber2, clBlack);
  strip.show();
  delay(param);
}

void flashRandomColor() {
  
  rndNumber1 = random(stripLed-1);
  strip.setPixelColor(rndNumber1, pgm_read_dword(&colorTab[random(96)]));
  rndNumber2 = random(stripLed-1);
  strip.setPixelColor(rndNumber2, pgm_read_dword(&colorTab[random(96)]));
  strip.show();
  delay(10);
  strip.setPixelColor(rndNumber1, clBlack);
  strip.setPixelColor(rndNumber2, clBlack);
  strip.show();
  delay(param);
}

void flashColor1() {
  TColor cl;
  
  strip.setPixelColor(rndNumber1, clBlack);
  strip.setPixelColor(rndNumber2, clBlack);
  cl.dw = pgm_read_dword(&colorTab[random(96)]);
  cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
  strip.setPixelColor(random(stripLed), cl.dw);
  cl.dw = pgm_read_dword(&colorTab[random(96)]);
  cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
  strip.setPixelColor(random(stripLed), cl.dw);
  rndNumber1 = random(stripLed);
  strip.setPixelColor(rndNumber1, clWhite);
  rndNumber2 = random(stripLed);
  strip.setPixelColor(rndNumber2, clWhite);
  strip.show();
  delay(50);
}

void flashColor2() {
  TColor cl;
  
  cl.dw = pgm_read_dword(&colorTab[random(96)]);
  cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
  strip.setPixelColor(rndNumber1, cl.dw);
  rndNumber1 = random(stripLed);
  strip.setPixelColor(rndNumber1, clWhite);
  strip.show();
  delay(50);
}

void flashColor3() {
  TColor cl;
  
  strip.setPixelColor(rndNumber1, clBlack);
  strip.setPixelColor(rndNumber2, clBlack);
  if (nPhase++ > 128) nPhase=0;
  if (nPhase > 96) {
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
    strip.setPixelColor(random(stripLed), cl.dw);
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
    strip.setPixelColor(random(stripLed), cl.dw);
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
    strip.setPixelColor(random(stripLed), cl.dw);
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    cl.r >>= 2; cl.g >>= 2; cl.b >>= 2;
    strip.setPixelColor(random(stripLed), cl.dw);
  }
  rndNumber1 = random(stripLed);
  strip.setPixelColor(rndNumber1, clWhite);
  rndNumber2 = random(stripLed);
  strip.setPixelColor(rndNumber2, clWhite);
  strip.show();
  delay(50);
}

void ColorWhite(uint8_t colorNumber) {
  uint8_t i;
  uint16_t br;
  TColor cl;
  if (colorNumber<48) {
    colorNumber *= 2;
    if (colorNumber>95) colorNumber=95; 
    cl.dw = pgm_read_dword(&colorTab[colorNumber]);
    cl.dw = strip.Color(cl.r, cl.g, cl.b);
    for(i=0; i<stripLed; i++) strip.setPixelColor(i, cl.dw);
  }
  else {
    br = 63 - colorNumber;
    br *= 16;
    if (br>239) colorNumber = 255;
    cl.dw=strip.Color(br, br, br);
    for(i=0; i<stripLed; i++) strip.setPixelColor(i, cl.dw);
  }
  strip.show();
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i;

    for(i=0; i< strip.numPixels(); i++) strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + progStep) & 255));
    strip.show();
    delay(param);
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) strip.setPixelColor(i+q, Wheel( (i+progStep) % 255));    //turn every third pixel on
      strip.show();
      delay(param);
      for (int i=0; i < strip.numPixels(); i=i+3) strip.setPixelColor(i+q, 0);        //turn every third pixel off
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setMode() {
  if (readData[20]<232) {
    prog = readData[20]>>3;
    subprog = readData[20] & 7;
  }
  else {
    prog = 29;
    subprog = readData[20]-232;
  }
  param = readData[21];
  brightness = readData[22];
  control = readData[23];
  paramTabl[prog]=param;
}

void clSet(uint8_t spr) {
  TDWord k;
  k.dw=pgm_read_dword(&progColor[spr]);
  clN[0].dw=pgm_read_dword(&colorTab[k.b0]);
  clN[1].dw=pgm_read_dword(&colorTab[k.b1]);
  clN[2].dw=pgm_read_dword(&colorTab[k.b2]);
  clN[3].dw=pgm_read_dword(&colorTab[k.b3]);
}

TColor mulsBrightness(TColor cl) {
  TWord akk;
   
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  return cl;
}

TColor mulsGain(TColor cl, uint8_t br) {
  TWord akk;
   
  akk.w = cl.r * br; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
  akk.w = cl.g * br; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
  akk.w = cl.b * br; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
  return cl;
}

void cmdMusical() {
      switch (subprog) {
         case 0: { zRainbow(); break; }    // 232
         case 1: { zMIX(); break; }
         case 2: { zIBeam(); break; }
         case 3: { zMidland(); break; }
         case 4: { zMagicFrom(); break; }
         case 5: { zMagicTo(); break; }
         case 6: { zFeeryFrom(); break; }
         case 7: { zFeeryTo(); break; }    // 239
         case 8: { zAllureFrom(); break; } // 240
         case 9: { zAllureTo(); break; }   // 241
         case 10: { skazka(); break; }     // 242
         case 11: { zLevel(); break; }     // 243
         case 12: { zIBeam2(); break; }    // 244
         case 13: { zMidland2(); break; }
         case 14: { zCharm(); break; }
         case 15: { zRainbow10(); break; } // 247
         case 20: { ColorWhite(param); strip.show(); break; } // 252 - установка цвета ленты
      }
}

void cmdRunning(uint8_t t) {

    progStep=0; 
    progSubStep=0;
 
    if (prog==1) {
       switch (subprog) {
         case 0: case 1: case 2: { sub00(); break; }
         case 3: case 4: case 5: { sub01(); break; }
         case 6: { sub06(); break; }
         case 7: { sub07(); break; }
       }
    }
    else {
      clSet(subprog);
      switch (prog) {
        case 0: {
          switch (subprog) {
             case 0: { theaterChaseRainbow(); break; }
             case 1: { rainbowCycle(); break; }
             case 2: { strip.clear(); flashRandom(); break; }
             case 3: { strip.clear(); flashRandomColor(); break; }
             case 4: { flashColor1(); break; }
             case 5: { flashColor2(); break; }
             case 6: { flashColor3(); break; }
             case 7: { flashColor3(); break; }
          }
          break; 
        }
        case 1: {
          switch (subprog) {
            case 0: case 1: case 2: { sub00(); break; }
            case 3: case 4: case 5: { sub01(); break; }
            case 6: { sub06(); break; }
            case 7: { sub07(); break; }
          }
        }
        case 2: { sub1(); break; };  
        case 3: { sub2(); break; };  
        case 4: { sub3(); break; };
        case 5: { sub4(); break; };  
        case 6: { sub5(); break; };  
        case 7: { sub6(); break; };  
        case 8: { sub7(); break; };  
        case 9: { sub8(); break; };  
        case 10: { sub9(); break; };
        case 11: { progSubStep=stripLed-1; sub10(); break; };
        case 12: { progSubStep=stripLed-1; sub11(); break; };
        case 13: { progSubStep=stripLed-1; sub12(); break; };
        case 14: { stroboscope(t); break; }
      }  
    }
}

//  SerialEvent
void serialEvent() {
  uint8_t i;
  
  while (Serial.available()) {
    // get the new byte:
    uint8_t inChar = (uint8_t)Serial.read();
    if (inChar != 253) {
      inStr[inCounter++] = inChar;
      if (inChar == 254) {
        i=0;
        while (inStr[i]!=254) {
          readData[i]=inStr[i];
          i++;
        }
        readData[i]=inStr[i];
        inCounter = 0;          // Устанавливаем счётчик принятых символов на начало входного буфера
        inComplete = true;
      }
    } else inCounter = 0;      // Устанавливаем счётчик принятых символов на начало входного буфера
  }
}

void loop(){

    if (inComplete) {
      inComplete = false;
      if (readData[20]<232) { setMode(); cmdRunning(param); }  
         else {
           if (readData[20]<255)  { setMode(); cmdMusical(); }
           else {
              /* Изменение параметров */
              param = readData[21];
              brightness = readData[22];
              control = readData[23];
              if (control>0) {
                if (prog>12) prog=0;
                rotateStep=pgm_read_byte(&rotateInterval[prog]);
              }            
           }
         }       
      Serial.println("OK"); // Подтверждение - команда выполнена
    }
    else {
      if (prog<29) {
        if ((control&1)!=0) {
          if (progStep==0) {
            if (++rotateStep==0) {
              prog=random(72);  
              subprog = prog & 7;
              prog >>= 3;
              param = paramTabl[prog];
              clSet(subprog);
              rotateStep=pgm_read_byte(&rotateInterval[prog]);
            }
          }
        }
        if (prog>0) {
          delay(param);
          switch (prog) {
            case 1: {
             switch (subprog) { 
              case 0: case 1: case 2: { sub00(); break; }
              case 3: case 4: case 5: { sub01(); break; }
              case 6: { sub06(); break; }
              case 7: { sub07(); break; }
             }
             break;
            }
            case 2: { sub1(); break; }
            case 3: { sub2(); break; }
            case 4: { sub3(); break; }
            case 5: { sub4(); break; }
            case 6: { sub5(); break; }
            case 7: { sub6(); break; }
            case 8: { sub7(); break; }
            case 9: { sub8(); break; }
            case 10: { sub9(); break; }
            case 11: { sub10(); break; }
            case 12: { sub11(); break; }
            case 13: { sub12(); break; }
            case 14: { stroboscope(param); progStep++; break; }
          }
        }
        else {
          switch (subprog) {
            case 0: { theaterChaseRainbow(); break; }
            case 1: { rainbowCycle(); break; }
            case 2: { flashRandom(); break; }
            case 3: { flashRandomColor(); break; }
            case 4: { flashColor1(); break; }
            case 5: { flashColor2(); break; }
            case 6: { flashColor3(); break; }
            case 7: { flashColor3(); break; }
          }
          progStep++;
        }
      }
    }
}
