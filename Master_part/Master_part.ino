#include <dht.h>

dht DHT_11;

#define temp 13
void setup() {
  Serial.begin(38400);
}

void transmitter()
 {
  int val=DHT_11.read11(temp);
  int humid=DHT_11.humidity;
  int temper=DHT_11.temperature;
  Serial.write(255);
  Serial.write(250);
  Serial.write(humid);
  Serial.write(temper);
  Serial.write(170);
 }

void loop() {
  transmitter();
}
