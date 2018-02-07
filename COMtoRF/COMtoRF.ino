#include <SPI.h>        // Подключаем библиотеку для работы с шиной SPI
#include <nRF24L01.h>   // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>       // Подключаем библиотеку для работы с nRF24L01+
RF24    radio(9, 10);   // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)

char    rfData[32];     // Создаём массив для передачи данных
char    serData[32];    // a string to hold incoming data
uint8_t rfCounter = 0;
uint8_t serCounter = 0;
boolean stringComplete = false;

void setup(){
    Serial.begin(115200);
    radio.begin();                             // Инициируем работу nRF24L01+
    radio.setChannel(5);                       // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_1MBPS);        // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);      // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openWritingPipe (0x1234567890LL);    // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
}

void loop(){
  uint8_t i;
  
    if (rfCounter>0) {
      radio.write(&rfData[0],rfCounter);
      Serial.println("ОК"); // Подтверждение - команда выполнена
//      Serial.write(rfData,rfCounter);
      rfCounter=0;
    }
}

//  SerialEvent
void serialEvent() {
  uint8_t i;
  
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar != char(253)) {
     if (inChar == char(254)) {
       for(i=0; i<serCounter; i++) rfData[i]=serData[i];
       rfCounter = serCounter;
       serCounter = 0;          // clear the input string:
     }
     else {
      serData[serCounter++] = inChar;
      serCounter &= 0x1F;
     }
    } else serCounter = 0;      // clear the input string: 
  }
}

