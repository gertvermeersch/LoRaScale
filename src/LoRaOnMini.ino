#include <SoftwareSerial.h>
#include <RN2483.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <HX711.h>

#define RESETPIN 4
#define DOUT3 2
#define CLK3 3

#define PERIOD 37 //approx. 5 min
#define calibration_factor_3 -217 //afwijking scale 3
#define calibration_factor_1 -214 //afwijking scale 1
#define DEBUG 1


HX711 scale1(DOUT3, CLK3);
SoftwareSerial softSerial(6, 7); //rx tx
RN2483 modem = RN2483(&Serial, &softSerial, RESETPIN);
char counter = 15;
char msgcounter = 0;
bool connected = false;
bool send = false;

void intro() {
  
  softSerial.println("-- LoRa scale --\n\r");
  softSerial.println("Author: Gert Vermeersch");
  softSerial.println("Date: 26/12/2017");
  // modem.setAppKey("136110f317d99f9f27808896778cca1a");
  // modem.setAppEUI("e0cf8b4d87a6ce01");
  // modem.setMacParam("dr", "0");
  // modem.saveMac();
  softSerial.print("deveui: ");
  softSerial.println(modem.getDeveui()); 
  softSerial.print("identifier: ");
  softSerial.println(modem.getDevAddress());
  softSerial.print("appeui: ");
  softSerial.println(modem.getAppEUI());
  
  
}

void connectToNetwork(short retries) {
  modem.setRadioParam("pwr", "15");
  
  softSerial.print("joining network...");
  short max_retries = retries;
  short tryc = 1;
  while(!modem.joinOTAA() && --retries >= 0) {
    char disp[19];
    sprintf(disp, "attempt %2d of %2d", tryc++, max_retries);
    softSerial.println(disp);
    delay(10000);
  }
}

void setup() {
  Serial.begin(57600); //timing sucks on softSerial
  softSerial.begin(57600);
  softSerial.print("init software serial connection...");
  
  softSerial.println("ok");
  
  softSerial.print("init HX711...");
  
   scale1.set_scale(calibration_factor_1);
   scale1.tare();
   softSerial.println("ok");
  //setup LoRa
  // softSerial.print("modem hard reset...");
  // pinMode(13, OUTPUT);
  // digitalWrite(13, HIGH);
  // put your setup code here, to run once:
  //manual reset
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(50);
  digitalWrite(9, HIGH); //drives transistor: powers modem
  softSerial.println("ok");
  
  softSerial.print("init modem...");
  modem.init();
  softSerial.println("ok");
  
  intro();

  connectToNetwork(10);

  softSerial.print("ok");
	delay(10);
  
  digitalWrite(13, LOW);
  
  softSerial.println("Join ok");
  byte payload[4] = {0xFF};
  modem.send(payload, 1, 1, true);  

    //setup sleep
  cli();
  wdt_reset();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDIE) | (0 << WDE) |
    (1 << WDP3) | (0 << WDP2) | (0 << WDP1) |
    (1 << WDP0);
  sei();
}

ISR(WDT_vect)
{

}

void enterSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   
  sleep_enable();
  sleep_mode();

  /* The program will continue from here after the WDT timeout*/
  sleep_disable();
  power_all_enable(); //TODO: check if we need to power all peripherals
}

void loop() {
  int measurement = int(abs(scale1.get_units(5) + 0.5));
  softSerial.print("Weight: ");
  softSerial.println(measurement);
  if (send) {
    counter = 0;
    digitalWrite(13, HIGH);
    //modem.setLed(true);
    delay(100);
    digitalWrite(13, LOW);
    //modem.setLed(false);
    delay(100);
    digitalWrite(13, HIGH);
    //modem.setLed(true);
    delay(100);
    digitalWrite(13, LOW);
    //modem.setLed(false);
      softSerial.println("Sending message");
      byte payload[3];
      payload[0] = 0xFF;
      payload[1] = measurement >> 8;
      payload[2] = measurement;
      payload[3] = ++msgcounter >> 8;
      payload[4] = msgcounter;
      modem.send(payload, 1, 3, false);
    send = false;
  }
  modem.handleUART();
  softSerial.println("going to sleep");
  delay(10);
  modem.handleUART();
  enterSleep();
  
  softSerial.println("woke up");
  
 

  if(counter++ == 15) {
    send = true;
    counter = 0;
  }
}
  





