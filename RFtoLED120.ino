#include <SPI.h>                   // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>              // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                  // Подключаем библиотеку  для работы с nRF24L01+
#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

#define stripPin 2     // выход управления светодиодной лентой
#define irPin 3        // вход IR

#define stripLed 120   // количество светодиодов в ленте
#define bandPass 20    // полос (групп светодиодов)
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
IRrecv irrecv(irPin);

decode_results results;

RF24      radio(9, 10);    // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
  
const uint32_t PROGMEM
  colorTab[]={
      0xFF0000,0xFF1100,0xFF2200,0xFF3300,0xFF4400,0xFF5500,0xFF6600,0xFF7700,0xFF8800,0xFF9900,0xFFAA00,0xFFBB00,0xFFCC00,0xFFDD00,0xFFEE00,0xFFFF00,  //красный - жёлтый
      0xFFFF00,0xEEFF00,0xDDFF00,0xCCFF00,0xBBFF00,0xAAFF00,0x99FF00,0x88FF00,0x77FF00,0x66FF00,0x55FF00,0x44FF00,0x33FF00,0x22FF00,0x11FF00,0x00FF00,  //жёлтый - зелёный
      0x00FF00,0x00FF11,0x00FF22,0x00FF33,0x00FF44,0x00FF55,0x00FF66,0x00FF77,0x00FF88,0x00FF99,0x00FFAA,0x00FFBB,0x00FFCC,0x00FFDD,0x00FFEE,0x00FFFF,  //зелёный - циан (голубой)
      0x00FFFF,0x00EEFF,0x00DDFF,0x00CCFF,0x00BBFF,0x00AAFF,0x0099FF,0x0088FF,0x0077FF,0x0066FF,0x0055FF,0x0044FF,0x0033FF,0x0022FF,0x0011FF,0x0000FF,  //голубой - синий
      0x0000FF,0x1100FF,0x2200FF,0x3300FF,0x4400FF,0x5500FF,0x6600FF,0x7700FF,0x8800FF,0x9900FF,0xAA00FF,0xBB00FF,0xCC00FF,0xDD00FF,0xEE00FF,0xFF00FF,  //синий - пурпур (маджента)
      0xFF00FF,0xFF00EE,0xFF00DD,0xFF00CC,0xFF00BB,0xFF00AA,0xFF0099,0xFF0088,0xFF0077,0xFF0066,0xFF0055,0xFF0044,0xFF0033,0xFF0022,0xFF0011,0xFF0000,  //маджента - красный
      0xFFFFFF,0xEFEFEF,0xDFDFDF,0xCFCFCF,0xBFBFBF,0xAFAFAF,0x9F9F9F,0x8F8F8F,0x7F7F7F,0x6F6F6F,0x5F5F5F,0x4F4F4F,0x3F3F3F,0x2F2F2F,0x1F1F1F,0x0F0F0F,0X000000};
       
      
typedef union{
    struct {
      uint8_t b,g,r,w;
    };
    uint32_t dw;
} TColor;

typedef union{
    struct {
      uint8_t b0,b1;
    };
    uint16_t w;
} TWord;      

uint8_t readData[24];           // Буфер команды
uint8_t prog = 0;
uint8_t subprog = 0;
uint8_t subcolor = 0;
uint8_t progStep = 0;
uint8_t progSubStep = 0;
uint8_t progDir = 0;
uint8_t param = 10;
uint8_t rotate = 1;
uint8_t rotateStep = 0;
uint8_t brightness = 255;
uint8_t paramTabl[32] = {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20};
uint32_t clN[4] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00};
uint8_t stepN[4] = {0,0,0,0};
uint8_t waitN[4] = {3,3,4,4};
uint8_t waitNc[4] = {3,3,4,4};
uint32_t clBlack; 
uint32_t clWhite; 
uint8_t bpData[20];
uint8_t magn[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
uint8_t threshold[20] = {192,192,192,192,192,160,160,160,128,128,128,112,112,112,96,96,96,80,80,80};  

void setup(){ 
    // initialize serial:
    Serial.begin(115200);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    irrecv.enableIRIn(); // Start the receiver

    clWhite=strip.Color(255, 255, 255);
    clBlack=strip.Color(0, 0, 0);

    radio.begin();                              // Инициируем работу nRF24L01+
//    radio.setAutoAck(false);

    radio.setChannel(5);                        // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_1MBPS);         // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);       // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openReadingPipe (1, 0x1234567890LL);  // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)

    radio.startListening  ();                   // Включаем приемник, начинаем прослушивать открытые трубы
}

uint32_t clOne(uint8_t spr) {
  return pgm_read_dword(&colorTab[spr * 16]);
}

void sub9() {
   TColor cl;
   TWord akk;
   uint8_t i;

   for(i=0; i<4; i++) {
     waitN[i]--;  
     if (waitN[i]==0){
       waitN[i]=waitNc[i];
       stepN[i]++;
       if (stepN[i]>stripLed-1) stepN[i]=0;
       cl.dw=clN[i];
       akk.w = cl.r * brightness; cl.r = akk.b1;
       akk.w = cl.g * brightness; cl.g = akk.b1;
       akk.w = cl.b * brightness; cl.b = akk.b1;
       if (i%2==0) strip.setPixelColor(stepN[i], cl.dw);
              else strip.setPixelColor(stripLed-1-stepN[i], cl.dw);
     }
   }
   strip.show();
}

void sub8() {
  TColor cl;
  TWord akk;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>19) progStep=0;
  if (progStep>16) cl.dw=clN[0];
              else cl.dw=clN[1]; 
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
}

void sub7() {
  TColor cl;
  TWord akk;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>19) progStep=0;
  if (progStep>17) cl.dw=clN[0];
              else cl.dw=clN[1]; 
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
}

void sub6() {
  TColor cl;
  TWord akk;

  if (progStep>stripLed-1) {
    progStep=0;
    progDir=!progDir;
  }
  if (progDir==0) cl.dw=clN[0];
             else cl.dw=clN[1];
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(progStep ,cl.dw);            
  strip.show();
}

void sub5() {
  TColor cl;
  TWord akk;
  uint8_t i;
  
  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>9) {
    progStep=0;
    progDir=!progDir;
  }
  if (progDir==0) cl.dw=clN[0];
             else cl.dw=clN[1];
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0 ,cl.dw);            
  strip.show();
}


void sub4() {
  TColor cl;

  if (progStep>19) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
  cl.dw=clN[0];
  sub4_1(cl);
  strip.show();
}

void sub4_1(TColor color) {
  TColor cl;
  TWord akk;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<10) cl.dw = strip.Color(color.r >> progStep, color.g >> progStep, color.b >> progStep);
              else cl.dw = strip.Color(color.r >> 19-progStep, color.g >> 19-progStep, color.b >> 19-progStep); 
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0, cl.dw);
}

void sub3() {
  TColor cl;

  if (progStep>23) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
  cl.dw=clN[0];
  sub3_1(cl);
  strip.show();
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
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0, cl.dw);
}

void sub2() {

  if (progStep>7) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
  sub2_1(clN[0]);
  strip.show();
}

void sub2_1(uint32_t color) {
  TColor cl;
  TWord akk;
  uint8_t i;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<4) {
    cl.dw=color;
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
  } else cl.dw=clBlack; 
  strip.setPixelColor(0, cl.dw);
}

void sub1() {

  if (progStep>16) {
    progStep=0;
    if (subprog==7) {
       clN[0]=clOne(subcolor++);
       if (subcolor>6) subcolor=0;
    }
  }
  sub1_1(clN[0]);
  strip.show();
}

void sub1_1(uint32_t color) {
  TColor cl;
  uint8_t i;
  TWord akk;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep<8) {
     if (progStep<2) cl.dw=clWhite;
                else cl.dw=color;
  }
     else {
      if (progStep<10) cl.dw=clWhite;
                  else cl.dw=clBlack; 
     }
  akk.w = cl.r * brightness; cl.r = akk.b1;
  akk.w = cl.g * brightness; cl.g = akk.b1;
  akk.w = cl.b * brightness; cl.b = akk.b1;
  strip.setPixelColor(0, cl.dw);
}
   
void sub00() {
  uint8_t i;
  TColor cl;
  TWord akk;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>subprog) progStep=0;
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
  } else cl.dw = clBlack;
  strip.setPixelColor(0, cl.dw);
  strip.show();
}

void sub01() {
  uint8_t i;
  TColor cl;
  TWord akk;

  for(i=stripLed/2-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  for(i=stripLed/2; i<stripLed; i++) strip.setPixelColor(i,strip.getPixelColor(i+1));
  if (progStep>subprog-3) progStep=0;
  if (progStep==0) {
    cl.dw = pgm_read_dword(&colorTab[random(96)]);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
  } else cl.dw = clBlack;
  strip.setPixelColor(0, cl.dw);
  strip.setPixelColor(stripLed-1, cl.dw);
  strip.show();
}

void sub06() {
  uint8_t i;
  TColor cl;
  TWord akk;

  for(i=stripLed-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  if (progStep>8) progStep=0;
  if (progStep==0) {
    if (++subcolor>6) subcolor=0;
    cl.dw=clOne(subcolor);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
    strip.setPixelColor(0, cl.dw);
  }
  strip.show();
}

void sub07() {
  uint8_t i;
  TColor cl;
  TWord akk;

  for(i=stripLed/2-1; i>0; i--) strip.setPixelColor(i,strip.getPixelColor(i-1));
  for(i=stripLed/2; i<stripLed-1; i++) strip.setPixelColor(i,strip.getPixelColor(i+1));
  if (progStep>8) progStep=0;
  if (progStep==0) {
    if (++subcolor>6) subcolor=0;
    cl.dw=clOne(subcolor);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
    strip.setPixelColor(0, cl.dw);
    strip.setPixelColor(stripLed-1, cl.dw);
  }
  strip.show();
}

void zmu1() {
  TColor cl;
  TWord akk;
  uint8_t i,k,n,gain;

  for(i=0; i<bandPass; i++) {
      akk.w=readData[i]*brightness;
      gain=akk.b1;
      if (magn[i]>fallspeed-1) magn[i] -= fallspeed;
                          else magn[i] = 0; 
      if (magn[i]<gain) magn[i]=gain;
                   else gain=magn[i]; 
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
      akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
      akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      n=i*LedtoColor;
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(n+k, cl.dw);
  }
  strip.show();
}

void zmu2() {
  TColor cl;
  TWord akk;
  uint8_t i,k,gain;

  for(i=0; i<bandPass; i++) {
      akk.w=readData[i]*brightness;
      gain=akk.b1;
      if (magn[i]>fallspeed-1) magn[i] -= fallspeed;
                 else magn[i] = 0; 
      if (magn[i]<gain) magn[i]=gain;
                else gain=magn[i]; 
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
      akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
      akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      for(k=0; k<LedtoColor; k++) strip.setPixelColor(i+k*bandPass, cl.dw);
  }
  strip.show();
}

void zmu3() {
  TColor cl;
  TWord akk;
  uint8_t n,i,k,gain;

  for(i=0; i<bandPass; i++) {
      akk.w=readData[i]*brightness;
      gain=akk.b1;
      if (magn[i]>fallspeed-1) magn[i] -= fallspeed;
                 else magn[i] = 0; 
      if (magn[i]<gain) magn[i]=gain;
                else gain=magn[i]; 
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
      akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
      akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      n=i*LedtoColor/2;
      for(k=0; k<LedtoColor/2; k++) {
        strip.setPixelColor(n+k, cl.dw);
        strip.setPixelColor(stripLed-(n+k)-1, cl.dw);
      }
  }
  strip.show();
}

void zmu4() {
  TColor cl;
  TWord akk;
  uint8_t n,i,k,gain;

  for(i=0; i<bandPass; i++) {
      akk.w=readData[i]*brightness;
      gain=akk.b1;
      if (magn[i]>fallspeed-1) magn[i] -= fallspeed-1;
                 else magn[i] = 0; 
      if (magn[i]<gain) magn[i]=gain;
                else gain=magn[i]; 
      cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
      akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
      akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
      akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
      cl.dw = strip.Color(cl.r, cl.g, cl.b);
      n=i*LedtoColor/2;
      for(k=0; k<LedtoColor/2; k++) {
        strip.setPixelColor(stripLed/2+n+k, cl.dw);
        strip.setPixelColor(stripLed/2-(n+k)-1, cl.dw);
      }
  }
  strip.show();
}

void zmu5() {
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
       if (magn[i]>0) magn[i]-=10;
      }
  }   
  for (i=0;i<bandPass;i++) {
    akk.w=magn[i]*brightness;
    uint8_t gain=akk.b1;
    cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
    akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
    akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
    akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
    cl.dw=strip.Color(cl.r,cl.g,cl.b);
    uint8_t n=i*LedtoColor/2;
    for(uint8_t k=0; k<LedtoColor/2; k++) {
      strip.setPixelColor(stripLed/2+n+k, cl.dw);
      strip.setPixelColor(stripLed/2-(n+k)-1, cl.dw);
    }
  }   
  strip.show();
}

void zmu5_() {
  TColor cl;
  TWord akk;
  uint8_t i;

  for (i=0;i<bandPass;i++) {
    if (readData[i]>threshold[i]) magn[i]=250;
      else {
       if (magn[i]>0) magn[i]-=10;
      }
  }   
  for (i=0;i<bandPass;i++) {
    akk.w=magn[i]*brightness;
    uint8_t gain=akk.b1;
    cl.dw = pgm_read_dword(&colorTab[96*i/bandPass]);
    akk.w = cl.r * gain; akk.w = akk.b1 * akk.b1; cl.r = akk.b1;
    akk.w = cl.g * gain; akk.w = akk.b1 * akk.b1; cl.g = akk.b1;
    akk.w = cl.b * gain; akk.w = akk.b1 * akk.b1; cl.b = akk.b1;
    cl.dw=strip.Color(cl.r,cl.g,cl.b);
    uint8_t n=i*LedtoColor/2;
    for(uint8_t k=0; k<LedtoColor/2; k++) {
      strip.setPixelColor(stripLed/2+n+k, cl.dw);
      strip.setPixelColor(stripLed/2-(n+k)-1, cl.dw);
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

void zmu6() {
  TColor cl;
  TWord akk;
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
  if (readData[k]<12) {
    clN[0] = strip.Color(32, 32, 32);
  }
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
    clN[0] = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(stripLed/2, clN[0]);
  strip.setPixelColor(stripLed/2-1, clN[0]);
  strip.show();
}

void zmu7() {
  TColor cl;
  TWord akk;
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
  if (readData[k]<12) clN[0] = strip.Color(32, 32, 32);
  else {
    cl.dw = pgm_read_dword(&colorTab[96*k/bandPass]);
    akk.w = cl.r * brightness; cl.r = akk.b1;
    akk.w = cl.g * brightness; cl.g = akk.b1;
    akk.w = cl.b * brightness; cl.b = akk.b1;
    clN[0] = strip.Color(cl.r, cl.g, cl.b);
  }
  strip.setPixelColor(0, clN[0]);
  strip.setPixelColor(stripLed-1, clN[0]);
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
  delay(param);
}

void flashRandom() {
  long randomNumber;
  TColor cl;
  
  randomNumber = random(stripLed-1);
  cl.dw = strip.Color(255, 255, 255);
  strip.setPixelColor(randomNumber, cl.dw);
  strip.show();
  delay(10);
  cl.dw = strip.Color(0, 0, 0);
  strip.setPixelColor(randomNumber, cl.dw);
  strip.show();
  delay(param);
}

void flashRandomColor() {
  long randomNumber,randomColor;
  TColor cl;
  
  randomNumber = random(stripLed-1);
  randomColor = random(96);
  cl.dw = pgm_read_dword(&colorTab[randomColor]);
  strip.setPixelColor(randomNumber, cl.dw);
  strip.show();
  delay(10);
  cl.dw = strip.Color(0, 0, 0);
  strip.setPixelColor(randomNumber, cl.dw);
  strip.show();
  delay(param);
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
  prog = readData[20]>>3;
  subprog = readData[20] & 7;
  param = readData[21];
  brightness = readData[22];
  rotate = readData[23];
  paramTabl[prog]=param;
}

void clSet(uint8_t spr) {
  switch (spr) {
    case 0: { clN[0]=pgm_read_dword(&colorTab[0]);  clN[1]=pgm_read_dword(&colorTab[16]); clN[2]=pgm_read_dword(&colorTab[64]); clN[3]=pgm_read_dword(&colorTab[80]); break; }
    case 1: { clN[0]=pgm_read_dword(&colorTab[16]); clN[1]=pgm_read_dword(&colorTab[32]); clN[2]=pgm_read_dword(&colorTab[64]); clN[3]=pgm_read_dword(&colorTab[48]); break; }
    case 2: { clN[0]=pgm_read_dword(&colorTab[32]); clN[1]=pgm_read_dword(&colorTab[64]); clN[2]=pgm_read_dword(&colorTab[16]); clN[3]=pgm_read_dword(&colorTab[48]); break; }
    case 3: { clN[0]=pgm_read_dword(&colorTab[48]); clN[1]=pgm_read_dword(&colorTab[0]);  clN[2]=pgm_read_dword(&colorTab[80]); clN[3]=pgm_read_dword(&colorTab[16]); break; }
    case 4: { clN[0]=pgm_read_dword(&colorTab[64]); clN[1]=pgm_read_dword(&colorTab[16]); clN[2]=pgm_read_dword(&colorTab[32]); clN[3]=pgm_read_dword(&colorTab[80]); break; }
    case 5: { clN[0]=pgm_read_dword(&colorTab[80]); clN[1]=pgm_read_dword(&colorTab[16]); clN[2]=pgm_read_dword(&colorTab[0]);  clN[3]=pgm_read_dword(&colorTab[48]); break; }
    case 6: { clN[0]=pgm_read_dword(&colorTab[96]); clN[1]=pgm_read_dword(&colorTab[64]); clN[2]=pgm_read_dword(&colorTab[48]); clN[3]=pgm_read_dword(&colorTab[16]); break; }
    case 7: { clN[0]=pgm_read_dword(&colorTab[96]); clN[1]=pgm_read_dword(&colorTab[0]);  clN[2]=pgm_read_dword(&colorTab[32]); clN[3]=pgm_read_dword(&colorTab[64]); break; }
  }
}
/*
0  - красный
16 - желтый
32 - зеленый
48 - аква
64 - синий
80 - фиолетовый
*/

void initBrightness() {
  TWord akk;
    
  for(uint8_t i=1; i<bandPass; i++) {
    akk.w =  readData[i] * brightness;
    readData[i] = akk.b1;
  }
}

void cmdMusical() {
      if (prog==30) {
        switch (subprog) {
           case 0: { zmu1();  break; }
           case 1: { zmu2(); break; }
           case 2: { zmu3(); break; }
           case 3: { zmu4(); break; }
           case 4: { zmu5(); break; }
           case 5: { zmu6(); break; }
           case 6: { zmu7(); break; }
        }
      }
}

void cmdRunning(uint8_t t) {

    if (prog==0) {
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
        case 1: { sub1(); break; };  
        case 2: { sub2(); break; };  
        case 3: { sub3(); break; };
        case 4: { sub4(); break; };  
        case 5: { sub5(); break; };  
        case 6: { sub6(); break; };  
        case 7: { sub7(); break; };  
        case 8: { sub8(); break; };  
        case 9: { sub9(); break; };
        case 29: {
          switch (subprog) {
             case 0: { theaterChaseRainbow(); break; }
             case 1: { rainbowCycle(); break; }
             case 2: { strip.clear(); flashRandom(); break; }
             case 3: { strip.clear(); flashRandomColor(); break; }
             case 4: { stroboscope(t); break; }
          }
          break; 
        }
      }  
    }
}

void processResult(unsigned long code) {

  switch(code)  {
    case 0xFF906F:
      if (param>0) param--;
      break;
    case 0xFFA857:  
      if (param<30) param++;
      break;
    case 0xFF9867:  //><  
      subprog++;
      subprog &= 7;
      break;
    case 0xFF6897: param = 0; break; //0
    case 0xFF30CF: param = 1; break; //1
    case 0xFF18E7: param = 2; break; //2
    case 0xFF7A85: param = 3; break; //3
    case 0xFF10EF: param = 4; break; //4
    case 0xFF38C7: param = 5; break; //5
    case 0xFF5AA5: param = 6; break; //6
    case 0xFF42BD: param = 7; break; //7
    case 0xFF4AB5: param = 8; break; //8
    case 0xFF52AD: param = 9; break; //9
  } 
  param=paramTabl[param];
}

void loop(){

    if(radio.available()){        // Если в буфере имеются принятые данные, то получаем номер трубы, по которой они пришли, по ссылке на переменную pipe
      radio.read(&readData, 24);  // Приём команды
      if (readData[20]<240) { setMode(); cmdRunning(param); }  
        else {
          switch (readData[20]) {
            case 251: { setMode(); ColorWhite(param); strip.show(); break; }
            case 252: {  // Изменение параметров
              param = readData[21];
              brightness = readData[22];
              rotate = readData[23];
              if (rotate>0) {
                if (prog>9) prog=0;
                switch (prog) { 
                  case 1: { rotateStep=224; break; }
                  case 2: { rotateStep=224; break; }
                  case 3: { rotateStep=224; break; }
                  case 4: { rotateStep=224; break; }
                  case 5: { rotateStep=224; break; }
                  case 6: { rotateStep=224; break; }
                  case 7: { rotateStep=224; break; }
                  case 8: { rotateStep=224; break; }
                  case 9: { rotateStep=224; break; }
                }
              }
              break; 
            }
            default:  { setMode(); cmdMusical(); }
          }       
        }       
    }
    else {
      if (irrecv.decode(&results)) {
//        Serial.println(results.value, HEX);
        processResult(results.value);
        irrecv.resume(); // Receive the next value
      } else {
        if (prog<30) {
          if (progStep==0) {
            if (++rotateStep==0) {
              if (rotate!=0) {
/*                
                if (++subprog>7) {
                  subprog=0;
                  if (++prog>9) prog=0;
                }
*/
                prog=random(72);  
                subprog = prog & 7;
                prog >>= 3;
                param = paramTabl[prog];
                clSet(subprog);
                switch (prog) { 
                  case 1: { rotateStep=224; break; }
                  case 2: { rotateStep=224; break; }
                  case 3: { rotateStep=224; break; }
                  case 4: { rotateStep=224; break; }
                  case 5: { rotateStep=224; break; }
                  case 6: { rotateStep=224; break; }
                  case 7: { rotateStep=224; break; }
                  case 8: { rotateStep=224; break; }
                  case 9: { rotateStep=224; break; }
                }
              }
            }
          }
        }
        if (prog<29) delay(param);
        progStep++;
        switch (prog) {
          case 0: {
           switch (subprog) { 
            case 0: case 1: case 2: { sub00(); break; }
            case 3: case 4: case 5: { sub01(); break; }
            case 6: { sub06(); break; }
            case 7: { sub07(); break; }
           }
           break;
          } 
          case 1: { sub1(); break; }
          case 2: { sub2(); break; }
          case 3: { sub3(); break; }
          case 4: { sub4(); break; }
          case 5: { sub5(); break; }
          case 6: { sub6(); break; }
          case 7: { sub7(); break; }
          case 8: { sub8(); break; }
          case 9: { sub9(); break; }
          case 29: {
            switch (subprog) {
               case 0: { theaterChaseRainbow(); break; }
               case 1: { rainbowCycle(); break; }
               case 2: { flashRandom(); break; }
               case 3: { flashRandomColor(); break; }
               case 4: { stroboscope(param); break; }
            }
            break; 
          }
        }
      }
    }
}



