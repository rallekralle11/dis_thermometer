#include <SmoothThermistor.h>
#include <ChuckPlex.h>
#include <EEPROM.h>
#include "charlie.h"

#define WAKEUP_GPIO GPIO_NUM_3
//#define EEPROM_SIZE 1


bool thermistor;
int mode = 4;
int temp;
int lasttemp;
int ext;
const int cykel = 500;
const int periodvaken = 4000;
const int termistorcykel = 10000;
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

SmoothThermistor externtemp(1, ADC_SIZE_12_BIT, 10000, 10000, 3950, 25, 3);

void setup() {

  EEPROM.begin(1);
  mode = EEPROM.read(0);
  mode = 3;

  pinMode(3, INPUT);

  externtemp.useAREF(true);

  esp_deep_sleep_enable_gpio_wakeup(1 << 3, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för sovläge som avaktivaras av 3 = 0

  ext = externtemp.temperature() + cal;
  temp = map(analogRead(0), 123, 1241, -40, 50) + cal; //kolla temp direkt

  if(ext < -40){thermistor = 0;}
  else{thermistor = 1;}

  Serial.begin(115200);
  while(!Serial){}


  while(!digitalRead(3)){} // ser till så knapp släpps redigt om den används för att sätta igång
  delay(300);
}

void loop() {

  if(mode == 0){inut();} //lägen
  if(mode == 1){utin();}
  if(mode == 2){inalltid();}
  if(mode == 3){utalltid();}
  if(mode == 4){insov();}

}



void config(){ //inställningar
  mil = millis();
  while(millis() < mil + periodvaken){
    if(!digitalRead(3)){
      while(!digitalRead(3)){}
      mode++;
      if(mode == 9){mode = 0;}
    }

    if(mode == 0){plexy.enable(5);}
  }
  EEPROM.write(0, mode);
  esp_deep_sleep_start();
}

void inut(){ //visar innetemperaturen innan utetemp
  mil = millis();
  while(millis() < mil + periodvaken){
    for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
      if(light[temp][x]){
      plexy.enable(x);
      }
    }
  }
  mil = millis();
  if(thermistor){
    while(millis() < mil + periodvaken){
      for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
        if(light[ext][x]){
        plexy.enable(x);
        }
      }
    }
  }
  esp_deep_sleep_start();
}

void utin(){ //visar utetemp först, om tillgänglig
  mil = millis();
  if(thermistor){
    while(millis() < mil + periodvaken){
      for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
        if(light[ext][x]){
        plexy.enable(x);
        }
      }
    }
  }
  mil = millis();
  while(millis() < mil + periodvaken){
    for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
      if(light[temp][x]){
      plexy.enable(x);
      }
    }
  }
  esp_deep_sleep_start();
}

void inalltid(){ //alltid igång, inne
  if(millis() - mil > cykel){
    mil = millis();

    temp = map(analogRead(0), 123, 1241, -40, 50) + cal;
    } //kolla temp ibland
    
  
  for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
    if(light[temp][x]){
    plexy.enable(x);
    }
  }
  if(!digitalRead(3)){ //byt till yttertemperatur när knappen trycks
    while(!digitalRead(3)){plexy.clear();}
    ext = externtemp.temperature() + cal;
    while(millis() < mil + periodvaken){
      for(int x = 1; x <= 90; x++){
        if(light[ext][x]){
        plexy.enable(x);
        }
      }
    }
  }
}

void utalltid(){ //alltid igång, ute

  if(analogRead(1) > 10){
    if(millis() - mil > termistorcykel){
      mil = millis();
    plexy.clear();
    ext = externtemp.temperature() + cal;} //kolla temp ibland

    }

  else{ //larma om termistorn inte är inkopplad
    plexy.enable(53);
    delay(500);
    plexy.clear();
    delay(500);
  }

    for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
      if(light[ext][x]){
      plexy.enable(x);
      }
    }


  if(!digitalRead(3)){ //byt till inne när knappen trycks
    while(!digitalRead(3)){plexy.clear();}
    temp = map(analogRead(0), 123, 1241, -40, 50) + cal;
    while(millis() < mil + periodvaken){
      for(int x = 1; x <= 90; x++){
        if(light[temp][x]){
        plexy.enable(x);
        }
      }
    }
  }
}

void insov(){ //på/av med knappen

  if(millis() - mil > cykel){
    mil = millis();

    temp = map(analogRead(0), 123, 1241, -40, 50) + cal;
  }

  for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
    if(light[temp][x]){
    plexy.enable(x);
    }
  }

  if(!digitalRead(3)){ //sov om knapp trycks
    while(!digitalRead(3)){plexy.clear();}
    delay(100);
    esp_deep_sleep_start();
  }
}