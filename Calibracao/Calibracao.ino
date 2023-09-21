//DIY_CHEAP_PERFECT

#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN =4;

HX711 scale;

//DIY_CHEAP_PERFECT

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();                // reset the scale to 0
}

//DIY_CHEAP_PERFECT

void loop() {
  
  if (scale.is_ready()) {
    Serial.print("HX711 reading: ");
    Serial.println(scale.get_value(10)); // print the average of 5 readings from the ADC minus the tare weight, set with tare()
  } else {
    Serial.println("HX711 not found.");
  }

  delay(500);
  
}

//DIY_CHEAP_PERFECT
