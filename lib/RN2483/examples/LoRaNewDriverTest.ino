#define DEBUG

#include <CurieTimerOne.h>

#include <RN2483.h>
#define RESETPIN 2


RN2483 modem = RN2483(&Serial1, RESETPIN);
int counter = 0;
bool connected = false;
bool send = false;

void setup() {
  pinMode(13, OUTPUT);
  // put your setup code here, to run once:
  Serial1.begin(57600);
  Serial.begin(57600);
  while (!Serial1 || !Serial);
  modem.init();
  //modem.setNWKSKey("YOUR KEY");
  //modem.setAppsKey("YOUR KEY");
  //modem.saveMac();
  //modem.setAppKey("YOUR KEY");
  //modem.saveMac();
  if (modem.joinABP()) {
    Serial.println("Join ok");
    connected = true;
    byte payload[4] = {0x01, 0x02, 0x03, 0x04};
    modem.send(payload, 1, 4, true);
  }

  CurieTimerOne.start(60000000, &interrupt);
}

void interrupt() {
  send = true; //enkel switch zetten -> serial wordt pas na interrupt verstuurd
}

void loop() {
  if (Serial.available()) {
    Serial.println("reading input from Serial");
    String msg = Serial.readStringUntil('\r');
    while (Serial.available()) {
      Serial.read();
    } //discard all following character
    Serial.print(">");
    Serial.println(msg);
    Serial1.print(msg);
    Serial1.print("\r\n");
    byte i = 0;
    while (!Serial1.available() && i++ < 100) {
      delay(1);
    }
    Serial.println(Serial1.readStringUntil('\n'));
  }
  if (send) {
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

    if (connected) {
      Serial.println("Sending message");
      byte payload[3];
      payload[0] = 0xFF;
      payload[1] = counter >> 8;
      payload[2] = counter;
      modem.send(payload, 1, 2, true);
    } else {
      Serial.println("Trying to connect...");
      connected = modem.joinABP();
    }
    send = false;
  }
  modem.handleUART();
}

