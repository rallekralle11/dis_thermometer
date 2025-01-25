/*
kom ihåg
testa batteriläsning
se till så den inte kan gå över eller under skalan
ev blinka max eller min
*/

#include <OneWire.h>
#include <DallasTemperature.h>
//#include <NonBlockingDallas.h>

#include "src/ChuckPlex-19a3b/ChuckPlex.h"
#include <Preferences.h>
#include "charlie.h"

#define WAKEUP_GPIO GPIO_NUM_2
#define ESP_SLEEP_GPIO_ENABLE_INTERNAL_RESISTORS false

//förbereder oneWire för temperatursensorerna
OneWire oneWire(8);
DallasTemperature sensors(&oneWire);
#define TEMPERATURE_PRECISION 9

int val;
int temp;
int ext;
int cfgmil;
unsigned long senast = 0;
const int cykel = 10000;
const int periodvaken = 3000;
const int periodvaken_inst = 10000;
const int urladdat = 2220;
unsigned long mil;
unsigned long milvakna;

Preferences minne; //förbered permanent minne

int pins[] = {3,21,4,5,6,7,0,9,10,20};
int nodes = 90;
ChuckPlex plexy = ChuckPlex(pins, 10);

void setup() {
  minne.begin("mode", false);
  val = minne.getInt("mode", 0); //ta värde från minne
  //val = 7;

  pinMode(2, INPUT);

  esp_deep_sleep_enable_gpio_wakeup(1 << 2, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för sovläge som avaktivaras av 2 = 0

  sensors.setWaitForConversion(false);
  sensors.begin();
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0); //kolla temp direkt
  ext = sensors.getTempCByIndex(1);

  if(analogRead(1) < urladdat){ //blinka om batteriet är lågt
    plexy.clear();
    pinMode(10, OUTPUT);
    pinMode(7, OUTPUT);
    digitalWrite(10, 0);
    analogWrite(7, 100);
    delay(500);
    analogWrite(7, 0);
  }

  /*Serial.begin(115200);
  while(!Serial){}

  Serial.println(temp);
  Serial.println(ext);*/

  cfgmil = millis();
  while(!digitalRead(2)){
    if(millis() > (cfgmil + periodvaken)){ //om knapp har hållts längre än periodvaken
      visa(0);
      config();
    }
  } // ser till så knapp släpps redigt om den används för att sätta igång
  delay(200);
}

void loop() {

  if(millis() > senast + periodvaken){//blinka om batteriet är lågt
    senast = millis();
    if(analogRead(1) < urladdat){
      plexy.clear();
      pinMode(10, OUTPUT);
      pinMode(7, OUTPUT);
      digitalWrite(10, 0);
      analogWrite(7, 100);
      delay(500);
      analogWrite(7, 0);
    }
  }

  //lägen
  //config();

  if(val == 0){inut();}//-20
  if(val == 1){utin();}//-10
  if(val == 2){insov();}//0
  if(val == 3){utsov();}//10
  if(val == 4){in();}//20
  if(val == 5){ut();}//30
  if(val == 6){intryck();}//40
  if(val == 7){uttryck();}//50
}

void config(){ //inställningar
  mil = millis();
  while(millis() < mil + periodvaken_inst){
    if(!digitalRead(2)){
      while(!digitalRead(2)){}
      delay(100);
      val++;
      if(val == 8){val = 0;}
      mil = millis(); //återställ tid när knapp trycks
    }

    if(val == 0){visa(-20);}
    if(val == 1){visa(-10);}
    if(val == 2){visa(0);}
    if(val == 3){visa(10);}
    if(val == 4){visa(20);}
    if(val == 5){visa(30);}
    if(val == 6){visa(40);}
    if(val == 7){visa(49);}
  }
  minne.putInt("mode", val);
}

void inut(){ //visar innetemperaturen innan utetemp
  mil = millis();
  while(millis() < mil + periodvaken){
    visa(temp);
    //Serial.println(temp);
  }

  if(ext > -50){
    mil = millis();
    while(millis() < mil + periodvaken){
      visa(ext);
    }
  }
  
  esp_deep_sleep_start();
}

void utin(){ //visar utetemp först, om tillgänglig
  mil = millis();

  if(ext > -50){
    while(millis() < mil + periodvaken){
      visa(ext);
    }
  }
  
  mil = millis();
  while(millis() < mil + periodvaken){
    visa(temp);
  }
  esp_deep_sleep_start();
}

void insov(){ //på/av med knappen

  if(millis() - mil > cykel){
    mil = millis();
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    Serial.println(temp);
  }

  visa(temp);

  if(!digitalRead(2)){ //sov om knapp trycks
    while(!digitalRead(2)){plexy.clear();}
    delay(100);
    esp_deep_sleep_start();
  }
  plexy.clear();
}

void utsov(){ //på/av med knappen

  if(ext > -50){
    if(millis() - mil > cykel){
      mil = millis();
      plexy.clear();
      sensors.requestTemperatures();
      ext = sensors.getTempCByIndex(1);
    } //kolla temp ibland

    }
  else{ //larma om termistorn inte är inkopplad
    fel();
  }

  visa(ext);

  visa(ext);

  if(!digitalRead(2)){ //sov om knapp trycks
    while(!digitalRead(2)){plexy.clear();}
    delay(100);
    esp_deep_sleep_start();
  }
  plexy.clear();
}

void in(){
  mil = millis();
  while(millis() < mil + periodvaken){
    visa(temp);
  }
  
  esp_deep_sleep_start();
}

void ut(){
  if(ext > -50){
    sensors.getTempCByIndex(0);
    ext = sensors.getTempCByIndex(1); //kolla temp
    while(millis() < mil + periodvaken){
      visa(ext);
    }
  }

  else{ //larma om termistorn inte är inkopplad
    plexy.enable(53);
    delay(500);
    plexy.clear();
    delay(500);
  }
  esp_deep_sleep_start();
}

void intryck(){
  if(millis() - mil > cykel){
    mil = millis();
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    Serial.println(temp);
  }

  visa(temp);

  if(!digitalRead(2)){
    mil = millis();
    while(!digitalRead(2)){
      plexy.clear();
      if(millis() > mil + periodvaken){ //sov om knapp hålls
        while(!digitalRead(2)){}
        esp_deep_sleep_start();
      }
      }
    delay(100);
    ext = sensors.getTempCByIndex(1);
    while(millis() < mil + periodvaken){
      visa(ext);
    }
  }
  plexy.clear();
}

void uttryck(){
  if(millis() - mil > cykel){
    mil = millis();
    sensors.requestTemperatures();
    ext = sensors.getTempCByIndex(1);
    temp = sensors.getTempCByIndex(0);
  }

  visa(ext);

  if(!digitalRead(2)){
    mil = millis();
    while(!digitalRead(2)){
      plexy.clear();
      if(millis() > mil + periodvaken){ //sov om knapp hålls
        while(!digitalRead(2)){}
        esp_deep_sleep_start();
      }
      }
    delay(100);
    while(millis() < mil + periodvaken){
      visa(temp);
    }
  }
  plexy.clear();
}

void visa(int y){
  y = y + 39;
  for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
    if(light[y][x]){
      plexy.enable(x);
    }
  }
}

void fel(){
    sensors.requestTemperatures();
    pinMode(10, OUTPUT);
    pinMode(7, OUTPUT);
    digitalWrite(10, 0);
    analogWrite(7, 100);
    delay(500);
    analogWrite(7, 0);
    delay(500);
    ext = sensors.getTempCByIndex(1);
}