#include "src/ChuckPlex-19a3b/ChuckPlex.h"
#include "src/SmoothThermistor/src/SmoothThermistor.h"
#include <Preferences.h>
#include "charlie.h"

#define WAKEUP_GPIO GPIO_NUM_3

int val;
int temp;
int ext;
const int cykel = 500;
const int periodvaken = 3000;
const int periodvaken_inst = 10000;
const int termistorcykel = 10000;
const int kal = 30; //prot 32
const int termkal = 35;
unsigned long mil;
unsigned long milvakna;

Preferences minne; //förbered permanent minne

int pins[] = {2,21,4,5,6,7,8,9,10,20};
int nodes = 90;
ChuckPlex plexy = ChuckPlex(pins, 10);

SmoothThermistor externtemp(1, ADC_SIZE_12_BIT, 10000, 10000, 3950, 25, 3);

void setup() {
  minne.begin("mode", false);
  val = minne.getInt("mode", 0); //ta värde från minne
  //mode = 0;

  pinMode(3, INPUT);

  externtemp.useAREF(true);

  esp_deep_sleep_enable_gpio_wakeup(1 << 3, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för sovläge som avaktivaras av 3 = 0

  ext = externtemp.temperature() + termkal;
  temp = map(analogRead(0), 123, 1241, -40, 50) + kal; //kolla temp direkt

  //Serial.begin(115200);
  //while(!Serial){}


  while(!digitalRead(3)){} // ser till så knapp släpps redigt om den används för att sätta igång
  delay(500);

  pinMode(20, INPUT_PULLUP);
  if(!digitalRead(20)){
    config();
  }
}

void loop() {

  //lägen
  if(val == 0){inut();} //0C
  if(val == 1){utin();} //10C
  if(val == 2){insov();} //20C
  if(val == 3){utsov();}//30C
  if(val == 4){in();} //40C
  if(val == 5){ut();} //50C
}

void config(){ //inställningar
  mil = millis();
  while(millis() < mil + periodvaken_inst){
    if(!digitalRead(3)){
      while(!digitalRead(3)){}
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
    ext = externtemp.temperature() + termkal;
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

    temp = map(analogRead(0), 123, 1241, -40, 50) + kal;
    } //kolla temp ibland
    
  
  visa(temp);

}

void utalltid(){ //alltid igång, ute

  if(analogRead(1) > 10){
    if(millis() - mil > termistorcykel){
      mil = millis();
      plexy.clear();
      ext = externtemp.temperature() + termkal;
    } //kolla temp ibland

    }

  else{ //larma om termistorn inte är inkopplad
    plexy.enable(53);
    delay(500);
    plexy.clear();
    delay(500);
  }

  visa(ext);

}

void insov(){ //på/av med knappen

  if(millis() - mil > cykel){
    mil = millis();

    temp = map(analogRead(0), 123, 1241, -40, 50) + kal;
  }

  visa(temp);

  if(!digitalRead(3)){ //sov om knapp trycks
    while(!digitalRead(3)){plexy.clear();}
    delay(100);
    esp_deep_sleep_start();
  }
  plexy.clear();
}

void utsov(){ //på/av med knappen

  if(millis() - mil > termistorcykel){
    mil = millis();

    ext = externtemp.temperature() + termkal;
  }

  visa(ext);

  if(!digitalRead(3)){ //sov om knapp trycks
    while(!digitalRead(3)){plexy.clear();}
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
    ext = externtemp.temperature() + termkal; //kolla temp
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