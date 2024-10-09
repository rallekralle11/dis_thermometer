#include <SmoothThermistor.h>

#include <ChuckPlex.h>

#include "charlie.h"

#define WAKEUP_GPIO GPIO_NUM_3


bool thermistor;
int mode = 0;
int temp;
int ext;
const int period = 1000;
const int periodvaken = 10000;
const int cal = 32;
unsigned long mil;
unsigned long milvakna;

//analog 0-4096
// /3,3=1241
//termometer -40-50 100-1000mv
//map(analogRead(), 123, 1241, -40, 50)

int pins[] = {2,21,4,5,6,7,8,9,10,20};
int nodes = 90;
ChuckPlex plexy = ChuckPlex(pins, 10);

SmoothThermistor externtemp(1, ADC_SIZE_12_BIT, 10000, 10000, 3950, 25, 5);

void setup() {

  pinMode(21, INPUT);
  pinMode(0, INPUT);

  externtemp.useAREF(true);

  esp_deep_sleep_enable_gpio_wakeup(1 << 3, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för sovläge som avaktivaras av 3 = 0

  ext = externtemp.temperature() + cal;
  temp = map(analogRead(0), 123, 1241, -40, 50) + cal; //kolla temp direkt

  if(ext < -100){thermistor = 0;}
  else{thermistor = 1;}

  Serial.begin(115200);
  while(!Serial){}
  Serial.println(thermistor);
}

void loop() {

  for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
    if(light[temp][x]){
      plexy.enable(x);
    }
  }

  if(millis() - mil > period){
    mil = millis();

    temp = map(analogRead(0), 123, 1241, -40, 50) + cal; //kolla temp ibland

    //Serial.println(temp);
    //Serial.println(ext);
  }


  if(millis() - milvakna > periodvaken){
    milvakna = millis();
    esp_deep_sleep_start();
  }
}