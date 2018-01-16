#include <Adafruit_NeoPixel.h>
 
#define ledPin 13       // светодиод на плате arduino
#define stripPin 2      // выход управления светодиодной лентой
#define stripLed 28     // количество светодиодов в ленте
#define bandPass 14     // число полос ЦМУ (используемых в программах)
#define ledDist 2
#define LedtoColor 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLed, stripPin, NEO_GRB + NEO_KHZ800);

const uint32_t PROGMEM
  colorTab[]={
      0xFF0000,0xFF1100,0xFF2200,0xFF3300,0xFF4400,0xFF5500,0xFF6600,0xFF7700,0xFF8800,0xFF9900,0xFFAA00,0xFFBB00,0xFFCC00,0xFFDD00,0xFFEE00,0xFFFF00,  //красный - жёлтый
      0xFFFF00,0xEEFF00,0xDDFF00,0xCCFF00,0xBBFF00,0xAAFF00,0x99FF00,0x88FF00,0x77FF00,0x66FF00,0x55FF00,0x44FF00,0x33FF00,0x22FF00,0x11FF00,0x00FF00,  //жёлтый — зелёный
      0x00FF00,0x00FF11,0x00FF22,0x00FF33,0x00FF44,0x00FF55,0x00FF66,0x00FF77,0x00FF88,0x00FF99,0x00FFAA,0x00FFBB,0x00FFCC,0x00FFDD,0x00FFEE,0x00FFFF,  //зелёный — циан (голубой)
      0x00FFFF,0x00EEFF,0x00DDFF,0x00CCFF,0x00BBFF,0x00AAFF,0x0099FF,0x0088FF,0x0077FF,0x0066FF,0x0055FF,0x0044FF,0x0033FF,0x0022FF,0x0011FF,0x0000FF,  //голубой — синий
      0x0000FF,0x1100FF,0x2200FF,0x3300FF,0x4400FF,0x5500FF,0x6600FF,0x7700FF,0x8800FF,0x9900FF,0xAA00FF,0xBB00FF,0xCC00FF,0xDD00FF,0xEE00FF,0xFF00FF,  //синий — пурпур (маджента)
      0xFF00FF,0xFF00EE,0xFF00DD,0xFF00CC,0xFF00BB,0xFF00AA,0xFF0099,0xFF0088,0xFF0077,0xFF0066,0xFF0055,0xFF0044,0xFF0033,0xFF0022,0xFF0011,0xFF0000}; //маджента — красный

typedef union{
    struct { uint8_t b,g,r,w; };
    uint32_t dw;
} TColor;

typedef union{
    struct { uint8_t b0,b1; };
    uint16_t w;
} TWord;

uint8_t inCounter = 0;
boolean inComplete = false;
uint8_t prog = 25;
uint8_t param = 63;
uint8_t progStep = 0;
uint8_t progDir = 0;
uint8_t dist = 0;
uint32_t clBlack; 
uint32_t clWhite; 
uint32_t clN[4] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00};
uint8_t stepN[4]  = {0,0,0,0};
uint8_t waitN[4]  = {3,3,4,4};
uint8_t waitNc[4] = {3,3,4,4};

uint8_t inStr[64];            
uint8_t readData[64];         

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

void loop() {
  // print the string when a newline arrives:
  if (inComplete) {
    inComplete = false;
    cmdExecute(readData[0], readData[1]);
    Serial.println("ОК"); // Подтверждение - команда выполнена
  }
  else {
    progStep++;
    switch (prog) {
        case 0: { sub0(param); break; }
        case 1: { sub1(param); break; } 
        case 2: case 3: case 4: { sub2(param); break; }
        case 5:  case 6:  case 7:  case 8:  case 9:  { sub5(param); break; }
        case 10: case 11: case 12: case 13: case 14: { sub10(param); break; }
        case 15: case 16: case 17: case 18: case 19: { sub15(param); break; }
        case 20: case 21: case 22: case 23: case 24: { sub20(param); break; }
        case 25: case 26: case 27: case 28: case 29: { sub25(param); break; }
        case 30: case 31: case 32: case 33: case 34: { sub30(param); break; }
        case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 44: { sub35(param); break; }
        case 45: case 46: case 47: case 48: case 49: { sub45(param); break; }
        case 50: case 51: { sub50(param); break; }
//      case 99: { sub21(param); break; }  // For future support
//      ......
        case 241: { theaterChaseRainbow(param); break; }
        case 242: { rainbowCycle(param); break; }
        case 243: { flashRandom(param); break; }
        case 244: { stroboscope(param); break; }
    }
  }
}

void setMode() {
  prog = readData[0];
  param = readData[1];
}

void clSet(uint8_t pr) {
  switch (pr) {
    case 2: { dist=0; break; }
    case 3: { dist=1; break; }
    case 4: { dist=2; break; }
    case 11:
    case 15:
    case 30:
    case 20:
    case 25: { clN[0]=strip.Color(255, 0, 0); clN[1]=strip.Color(0, 0, 255); break; }
    case 6:
    case 12:
    case 21:
    case 26: { clN[0]=strip.Color(255, 255, 0); clN[1]=strip.Color(0, 0, 255); break; }
    case 7:
    case 16:
    case 31:
    case 13: { clN[0]=strip.Color(0, 255, 0); break; }
    case 8:
    case 18:
    case 33: { clN[0]=strip.Color(0, 255, 255); break; }
    case 9:
    case 17:
    case 32:
    case 14: { clN[0]=strip.Color(0, 0, 255); break; }
    case 10:
    case 19:
    case 34: { clN[0]=strip.Color(255, 255, 255); break; }
    case 22:
    case 27: { clN[0]=strip.Color(255, 0, 127); clN[1]=strip.Color(127, 255, 0); break; }
    case 23:
    case 28: { clN[0]=strip.Color(0, 255, 127); clN[1]=strip.Color(127, 255, 0); break; }
    case 24:
    case 29: { clN[0]=strip.Color(127, 0, 255); clN[1]=strip.Color(0, 255, 127); break; }
    case 35: { clN[0]=strip.Color(255, 255, 255); clN[1]=strip.Color(255, 0, 0); break; }
    case 36: { clN[0]=strip.Color(255, 255, 255); clN[1]=strip.Color(0, 255, 0); break; }
    case 37: { clN[0]=strip.Color(255, 255, 255); clN[1]=strip.Color(0, 0, 255); break; }
    case 38: { clN[0]=strip.Color(255, 255, 255); clN[1]=strip.Color(0, 255, 255); break; }
    case 39: { clN[0]=strip.Color(255, 255, 255); clN[1]=strip.Color(255, 0, 255); break; }
    case 40: case 45: { clN[0]=strip.Color(255, 0, 0); clN[1]=strip.Color(0, 255, 255); break; }
    case 41: case 46: { clN[0]=strip.Color(255, 0, 0); clN[1]=strip.Color(255, 255, 255); break; }
    case 42: case 47: { clN[0]=strip.Color(0, 0, 255); clN[1]=strip.Color(255, 255, 0); break; }
    case 43: case 48: { clN[0]=strip.Color(0, 255, 0); clN[1]=strip.Color(255, 0, 255); break; }
    case 44: case 49: { clN[0]=strip.Color(255, 255, 0); clN[1]=strip.Color(0, 0, 255); break; }
    case 50: {
      clN[0]=strip.Color(255, 0, 0);
      clN[1]=strip.Color(0, 255, 0);
      clN[2]=strip.Color(0, 0, 255);
      clN[3]=strip.Color(255, 255, 0);
      break;
    }
    case 51: {
      clN[0]=strip.Color(255, 127, 0);
      clN[1]=strip.Color(0, 255, 127);
      clN[2]=strip.Color(127, 0, 255);
      clN[3]=strip.Color(128, 128, 128);
      break;
    }
    
  }
}

void cmdExecute(uint8_t p, uint8_t t) {

    switch (p) {
      case 0: { setMode(); sub0(t); break; }
      case 1: { setMode(); sub1init(); sub1(t); break; }
      case 2: case 3: case 4: { setMode(); clSet(p); sub2(t); break; }
      case 5:  case 6:  case 7:  case 8: case 9: { setMode(); clSet(p); sub5(t); break; }
      case 10: case 11: case 12: case 13: case 14: { setMode(); clSet(p); sub10(t); break; }
      case 15: case 16: case 17: case 18: case 19: { setMode(); clSet(p); sub15(t); break; }
      case 20: case 21: case 22: case 23: case 24: { setMode(); clSet(p); sub20(t); break; }
      case 25: case 26: case 27: case 28: case 29: { setMode(); clSet(p); sub25(t); break; }
      case 30: case 31: case 32: case 33: case 34: { setMode(); clSet(p); sub30(t); break; }
      case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 44: { setMode(); clSet(p); sub35(t); break; }
      case 45: case 46: case 47: case 48: case 49: { setMode(); clSet(p); sub45(t); break; }
      case 50: case 51: { setMode(); clSet(p); sub50(t); break; }
//      case 99: { setMode(); clSet(p); sub99(t); break; }  // For future support
//    ..................
      case 240: { setMode(); ColorWhite(t); break; }
      case 241: { setMode(); theaterChaseRainbow(t); break; }
      case 242: { setMode(); rainbowCycle(t); break; }
      case 243: { setMode(); flashRandom(t); break; }
      case 244: { setMode(); stroboscope(t); break; }
      case 245: { param = t; break; }  
      case 246: { strip.clear(); strip.show(); break; }
      case 247: { setMode(); zmu1();  break; }
      case 248: { setMode(); zmu2(); break; }
      case 249: { setMode(); zmu3(); break; }
//      case 250: { setMode(); zmu4(); break; } // For future support
//      case 251: { setMode(); zmu5(); break; } // For future support
//      case 252: { setMode(); zmu6(); break; } // For future support
    }
}

void sub50(uint8_t wait) {
   TColor cl;
   uint8_t i;

   for(i=0; i<4; i++) {
     waitN[i]--;  
     if (waitN[i]==0){
       waitN[i]=waitNc[i];
       stepN[i]++;
       if (stepN[i]>stripLed-1) stepN[i]=0;
       if (i%2==0) strip.setPixelColor(stepN[i], clN[i]);
              else strip.setPixelColor(stripLed-1-stepN[i], clN[i]);    
     }
   }
   strip.show();
   delay(wait);
}

void sub45(uint8_t wait) {
  uint32_t cl;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>19) progStep=0;
  if (progStep>16) strip.setPixelColor(0 ,clN[0]);
              else strip.setPixelColor(0 ,clN[1]); 
  strip.show();
  delay(wait);
}

void sub35(uint8_t wait) {
  uint32_t cl;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>19) progStep=0;
  if (progStep>17) strip.setPixelColor(0 ,clN[0]);
              else strip.setPixelColor(0 ,clN[1]); 
  strip.show();
  delay(wait);
}

void sub30(uint8_t wait) {
  TColor cl;

  if (progStep>stripLed-1) progStep=0;
  cl.dw=clN[0];
  sub30_1(cl);
  strip.show();
  delay(wait);
}

void sub30_1(TColor color) {
  uint32_t cl;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>19)  progStep=0;
  if (progStep<10) cl = strip.Color(color.r >> progStep, color.g >> progStep, color.b >> progStep);
              else cl = strip.Color(color.r >> 19-progStep, color.g >> 19-progStep, color.b >> 19-progStep); 
  strip.setPixelColor(0, cl);
}

void sub25(uint8_t wait) {
  uint32_t cl;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>19) progStep=0;
  if (progStep>9) strip.setPixelColor(0 ,clN[0]);
             else strip.setPixelColor(0 ,clN[1]); 
  strip.show();
  delay(wait);
}

void sub20(uint8_t wait) {
  
  if (progStep>stripLed-1) {
    progStep=0;
    progDir=!progDir;
  }
  if (progDir==0) strip.setPixelColor(progStep, clN[0]);
             else strip.setPixelColor(progStep, clN[1]);
  strip.show();
  delay(wait);
}

void sub15(uint8_t wait) {
  TColor cl;

  if (progStep>stripLed-1) progStep=0;
  cl.dw=clN[0];
  sub15_1(cl);
  strip.show();
  delay(wait);
}

void sub15_1(TColor color) {
  uint32_t cl;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>24)  progStep=0;
  if (progStep<10) {
    if (progStep<2) {
      if (progStep==0) cl = strip.Color(color.r >> 2, color.g >> 2, color.b >> 2);
                  else cl = strip.Color(color.r, color.g, color.b);
    } else cl = strip.Color(color.r >> (progStep-2), color.g >> (progStep-2), color.b >> (progStep-2));
  } else cl = clBlack; 
  strip.setPixelColor(0, cl);
}

void sub10(uint8_t wait) {

  if (progStep>stripLed-1) progStep=0;
  sub10_1(clN[0]);
  strip.show();
  delay(wait);
}

void sub10_1(uint32_t color) {
  uint32_t cl;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>7) progStep=0;
  if (progStep<4) cl=color;
             else cl=clBlack; 
  strip.setPixelColor(0, cl);
}

void sub5(uint8_t wait) {

  if (progStep>stripLed-1) progStep=0;
  sub5_1(clN[0]);
  strip.show();
  delay(wait);
}

void sub5_1(uint32_t color) {
  uint32_t cl;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) {
    cl=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl);
  }
  if (progStep>16)  progStep=0;
  if (progStep<8) {
     if (progStep<2) cl=clWhite;
                else cl=color;
  }
     else {
      if (progStep<10) cl=clWhite;
                  else cl=clBlack; 
     }      
  strip.setPixelColor(0, cl);
}
   
void sub2(uint8_t wait) {
  uint8_t i;
  TColor cl;

  for(i=stripLed-1; i>0; i--) {
    cl.dw=strip.getPixelColor(i-1);
    strip.setPixelColor(i,cl.dw);
  }
  if (progStep>dist) progStep=0;
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(95)]);
    cl.dw = strip.Color(cl.r, cl.g, cl.b); 
  } else cl.dw = strip.Color(0, 0, 0);
  strip.setPixelColor(0, cl.dw);
  strip.show();
  delay(wait);
}

void sub1init() {
  uint8_t i;
  TColor cl;
  
  strip.clear();
  i=stripLed-1;
  while(i>4) {
    cl.dw=pgm_read_dword(&colorTab[random(95)]);
    cl.dw = strip.Color(cl.r, cl.g, cl.b); 
    strip.setPixelColor(i,cl.dw);
    i -= 5;
  }
  strip.show();
}

void sub1(uint8_t wait) {
  uint8_t i;
  TColor cl;

  i=stripLed-1;
  while(i>4) {
    cl.dw=strip.getPixelColor(i-5);
    strip.setPixelColor(i,cl.dw);
    i -= 5;
  }
  if (progStep>dist) progStep=0;
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(95)]);
    cl.dw = strip.Color(cl.r, cl.g, cl.b); 
  } else cl.dw = strip.Color(0, 0, 0);
  strip.setPixelColor(i, cl.dw);
  strip.show();
  delay(wait);
}

void sub0(uint8_t wait) {
  TColor cl;
  
  if (progStep>stripLed-1) progStep=0;
  uint8_t randomNumber = random(95);
  cl.dw = pgm_read_dword(&colorTab[randomNumber]);
  cl.dw = strip.Color(cl.r, cl.g, cl.b);
  strip.setPixelColor(progStep, cl.dw);
  strip.show();
  delay(wait);
}

void zmu3() {
  TColor cl;
  TWord akk;
  uint8_t i,k,j,n;

  for(i=0; i<15; i++) {
      cl.dw = pgm_read_dword(&colorTab[96*i/15]);
      j=readData[i+2];
      akk.w = cl.r * j;
      akk.w = akk.b1 * akk.b1; 
      cl.r = akk.b1;
      akk.w = cl.g * j;
      akk.w = akk.b1 * akk.b1; 
      cl.g = akk.b1;
      akk.w = cl.b * j;
      akk.w = akk.b1 * akk.b1; 
      cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      n=i*ledDist/2;
      for(k=0; k<ledDist/2; k++) {
        strip.setPixelColor(n+k, cl.dw);
        strip.setPixelColor(stripLed-(n+k)-1, cl.dw);
      }
  }
  strip.show();
}

void zmu2() {
  TColor cl;
  TWord akk;
  uint8_t i,k,j;

  for(i=0; i<bandPass; i++) {
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      j=readData[i+2];
      akk.w = cl.r * j;
      akk.w = akk.b1 * akk.b1; 
      cl.r = akk.b1;
      akk.w = cl.g * j;
      akk.w = akk.b1 * akk.b1; 
      cl.g = akk.b1;
      akk.w = cl.b * j;
      akk.w = akk.b1 * akk.b1; 
      cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(i+k*bandPass, cl.dw);
  }
  strip.show();
}

void zmu1() {
  TColor cl;
  TWord akk;
  uint8_t n,i,k,j;

  for(i=0; i<bandPass; i++) {
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      j=readData[i+2];
      akk.w = cl.r * j;
      akk.w = akk.b1 * akk.b1; 
      cl.r = akk.b1;
      akk.w = cl.g * j;
      akk.w = akk.b1 * akk.b1; 
      cl.g = akk.b1;
      akk.w = cl.b * j;
      akk.w = akk.b1 * akk.b1; 
      cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      n=i*ledDist;
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(n+k, cl.dw);
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
  delay(wait);
}

void flashRandom(uint8_t wait) {
  long rn1,rn2;
  TColor cl;
  // Напишите свой код
  rn1 = random(stripLed-1);
  rn2 = random(stripLed-1);
  cl.dw = strip.Color(255, 255, 255);
  strip.setPixelColor(rn1, cl.dw);
  strip.setPixelColor(rn2, cl.dw);
  strip.show();
  delay(10);
  cl.dw = strip.Color(0, 0, 0);
  strip.setPixelColor(rn1, cl.dw);
  strip.setPixelColor(rn2, cl.dw);
  strip.show();
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
    strip.show();
  }
  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i;

    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + progStep) & 255));
    }
    strip.show();
    delay(wait);
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+progStep) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
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

