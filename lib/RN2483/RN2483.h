#ifndef RN2483_h
#define RN2483_h
#define TIMEOUT 2000 //default time out for expectString
#define BUFFERSIZE 100

#include <Stream.h>
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class RN2483
{
public:
  RN2483(Stream* stream, Stream* debugstream, uint8_t resetPin);
  void init();
  void reset();
  void autoBaud();
  bool joinABP();
  bool joinOTAA();
  bool setDevAddress(char* devAddress);
  bool setAppKey(char* appKey);
  bool setAppsKey(char* appsKey);
  bool setNWKSKey(char*  nwksKey);
  bool setDeveui(char* deveui);
  bool setAppEUI(char* appEUI);
  char* getDevAddress();
  char* getDeveui();
  char* getAppEUI();
  void pauseMac(void);
  void resumeMac(void);
  bool send(byte* payload, uint8_t port, uint8_t len, bool cnf = false);
  bool setMacParam(char* param, char* value);
  bool setRadioParam(char* param, char* value);
  char* getMacParam(char* param);
  bool saveMac(void);
  void handleUART(void);
  void setLed(bool);
  void resetCounter();


private:

  Stream* _debugstream;
	Stream* _stream;
	uint8_t _resetPin;
  uint8_t cnfcounter;
  char buffer[BUFFERSIZE];
	bool expectString(const char*, unsigned short timeout = TIMEOUT);
  uint8_t readBufferFromModem(unsigned short timeout = 1);
  void clearBufferFromModem();
};

#endif
