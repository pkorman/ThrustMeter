
#define VERSION_STR_1 "  Thrust Meter  "
#define VERSION_STR_2 " PK v26.11.2017 "

#define HEARTBEAT_PIN 13

// include the library code:
#include <HX711_ADC.h>
#include <LiquidCrystal.h>
#include <Bounce2.h>

// LCD: RS, Enable, D4, D5, D6, D7, R/W pin to ground, VSS pin to ground, VCC pin to 5V, http://www.arduino.cc/en/Tutorial/LiquidCrystal
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

//HX711 constructor (dout pin, sck pin) https://github.com/olkal/HX711_ADC
HX711_ADC LoadCell(2, 3);

//Bounce2 https://github.com/thomasfredericks/Bounce2
#define BUTTON_PIN_1 10
#define BUTTON_PIN_2 11

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

long measure_time;
float max_i;
boolean disp_val;
boolean snapshot;
byte snap_position;
long snap_time;

void setup() {
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0,0);
  lcd.print(VERSION_STR_1);
  lcd.setCursor(0, 1);
  lcd.print(VERSION_STR_2);

  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(492.0); // user set calibration factor (float)

  pinMode(BUTTON_PIN_1,INPUT_PULLUP);
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(5); // interval in ms
  
  pinMode(BUTTON_PIN_2,INPUT_PULLUP);
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(5); // interval in ms

  // initialize digital pin 13 as an output. heartbeat HEARTBEAT_PIN
  pinMode(HEARTBEAT_PIN, OUTPUT);

  
  lcd.clear();
  disp_val = true;
  snapshot = true;
  snap_position = 0;
}

void loop() {

  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in scetch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();

  // Update the Bounce instances :
  debouncer1.update();
  debouncer2.update();

  // Get the updated value :
  int value1 = debouncer1.read();
  int value2 = debouncer2.read();

  //SNAPSHOT funkcia
  if (value1 == LOW && snapshot && (millis() > snap_time + 1000)) {
    lcd.setCursor(snap_position, 1);
    lcd.print(String(LoadCell.getData(), 0));

    snap_position = snap_position + 6;

    if (snap_position > 12) {
       snapshot = false;
    }
    snap_time = millis();
  }

  //TARE funkcia
  if (value2 == LOW) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("***Tare***");
    max_i = 0;
    disp_val = false;
    snapshot = false;
    LoadCell.tareNoDelay();
  }

  //get smoothed value from data set + current calibration factor
  if ((millis() > measure_time + 125) && disp_val ) {
    float i = LoadCell.getData();
    if (i > max_i) {
      max_i = i;
      lcd.setCursor(12, 0);
      lcd.print("    ");
      lcd.setCursor(8, 0);
      lcd.print("max:" + String(max_i, 0));
    }

    lcd.setCursor(0, 0);
    lcd.print(String(i, 1) + "g ");
        
    measure_time = millis();

    digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    lcd.clear();
    disp_val = true;
    snapshot = true;
    snap_position = 0;
  }
  
}

