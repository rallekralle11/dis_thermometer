#include "charlie.h"
#include "driver/rtc_io.h"

#define WAKEUP_GPIO GPIO_NUM_3

int mode = 0;
int temp;
int period = 1000;
unsigned long mil;

//analog 0-4096
// /3,3=1241
//termometer -40-50 100-1000mv
//map(analogRead(), 123, 1241, -40, 50)

void setup() {
  // put your setup code here, to run once:
  pinMode(21, INPUT);

  esp_deep_sleep_enable_gpio_wakeup(1 << 3, ESP_GPIO_WAKEUP_GPIO_LOW); //förbered för såvläge som avaktivaras av 3 = 0
}

void loop() {
  // put your main code here, to run repeatedly:

      if(millis() - mil > period){
        mil = millis();

        temp = map(analogRead(0), 123, 1241, -40, 50); //kolla temp var sekund
      }
      



}