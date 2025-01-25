#include <OneWire.h>
#include <DallasTemperature.h>

#include "src/ChuckPlex-19a3b/ChuckPlex.h"
#include <Preferences.h>
#include "charlie.h"

#define WAKEUP_GPIO GPIO_NUM_2
#define ESP_SLEEP_GPIO_ENABLE_INTERNAL_RESISTORS false

#define ONE_WIRE_BUS 8 //förbereder oneWire för temperatursensorerna
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int val;
int temp;
int ext;
const int cykel = 500;
const int periodvaken = 3000;
const int periodvaken_inst = 10000;
const int termistorcykel = 10000;
unsigned long mil;
unsigned long milvakna;

Preferences minne; //förbered permanent minne

int pins[] = {3,21,4,5,6,7,0,9,10,20};
int nodes = 90;
ChuckPlex plexy = ChuckPlex(pins, 10);

void setup() {
  minne.begin("mode", false);
  val = minne.getInt("mode", 0); //ta värde från minne
  val = 2;

  pinMode(2, INPUT);

  esp_deep_sleep_enable_gpio_wakeup(1 << 2, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för sovläge som avaktivaras av 2 = 0


  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0); //kolla temp direkt
  ext = sensors.getTempCByIndex(1);

  //Serial.begin(115200);
  //while(!Serial){}


  while(!digitalRead(2)){} // ser till så knapp släpps redigt om den används för att sätta igång
  delay(200);
}

void loop() {

for(int t = 0; t != 89; t++){
  visa(t);
}
for(int t = 89; t != 0; t--){
  visa(t);
}
}

void config(){ //inställningar
  mil = millis();
  while(millis() < mil + periodvaken_inst){
    if(!digitalRead(2)){
      while(!digitalRead(2)){}
      delay(100);
      val++;
      if(val == 6){val = 0;}
      mil = millis(); //återställ tid när knapp trycks
    }

    if(val == 0){visa(39);}
    if(val == 1){visa(49);}
    if(val == 2){visa(59);}
    if(val == 3){visa(69);}
    if(val == 4){visa(79);}
    if(val == 5){visa(88);}
  }
  minne.putInt("mode", val);
}

void inut(){ //visar innetemperaturen innan utetemp
  mil = millis();
  while(millis() < mil + periodvaken){
    visa(temp);
  }
  if(analogRead(1) > 10){
    mil = millis();
    sensors.requestTemperatures();
    ext = sensors.getTempCByIndex(1);
  } //kolla temp ibland
    while(millis() < mil + periodvaken){
      visa(ext);
    }
  
  esp_deep_sleep_start();
}

void utin(){ //visar utetemp först, om tillgänglig
  mil = millis();
  if(analogRead(1) > 10){
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

void inalltid(){ //alltid igång, inne
  if(millis() - mil > cykel){
    mil = millis();

    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    } //kolla temp ibland
    
  visa(temp);
}

void utalltid(){ //alltid igång, ute

  if(analogRead(1) > 10){
    if(millis() - mil > termistorcykel){
      mil = millis();
      plexy.clear();
      sensors.requestTemperatures();
      ext = sensors.getTempCByIndex(1);
    } //kolla temp ibland

    }

  visa(ext);

}

void insov(){ //på/av med knappen

  if(millis() - mil > cykel){
    mil = millis();
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
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

  if(millis() - mil > termistorcykel){
    mil = millis();

    sensors.getTempCByIndex(0);
    ext = sensors.getTempCByIndex(1);
  }

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
  if(analogRead(1) > 10){
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

void visa(int y){
  for(int x = 1; x <= 90; x++){  //tänd lamporna för den nuvarande temperaturen
    if(light[y][x]){
      plexy.enable(x);
    }
  }
}