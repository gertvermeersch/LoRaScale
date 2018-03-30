#include "RN2483.h"
#define DEBUG 1

RN2483::RN2483(Stream* stream, Stream* debugstream, uint8_t resetPin) {
	_resetPin = resetPin;
	_stream = stream;
	_debugstream = debugstream;
	pinMode(_resetPin, OUTPUT);
}

void RN2483::init() {
	pinMode(_resetPin, OUTPUT);
	reset();
	clearBufferFromModem();
	
}

void RN2483::autoBaud() {
	//Triggers the auto baud detection as described in datasheet
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);
	delay(100);
	digitalWrite(1, HIGH);
	_stream->write(0x55);
	delay(1);
	clearBufferFromModem();
}

void RN2483::reset(void) {

	digitalWrite(_resetPin, LOW);
	delay(2000); //reset module
	digitalWrite(_resetPin, HIGH);
	delay(1000);
	clearBufferFromModem();
}

bool RN2483::setDevAddress(char* devAddress) {
	
	char param[] = "devaddr";
	return setMacParam(param, devAddress);
}

bool RN2483::setDeveui(char* deveui) {
	
	char param[] = "deveui";
	return setMacParam(param, deveui);
}

bool RN2483::setAppKey(char* appKey) {
	
	char param[] = "appkey";
	return setMacParam(param, appKey);
}

bool RN2483::setAppsKey(char* appKey) {
	
	char param[] = "appskey";
	return setMacParam(param, appKey);
}

bool RN2483::setNWKSKey(char*  nwksKey) {
	
	char param[] = "nwkskey";
	return setMacParam(param, nwksKey);
}

bool RN2483::setAppEUI(char* appEui) {
	
	char param[] = "appeui";
	return setMacParam(param, appEui);
}

char* RN2483::getDevAddress() {
	
	return getMacParam("devaddr");
}


  char* RN2483::getDeveui() {
	 
	  return getMacParam("deveui");
  }
  char* RN2483::getAppEUI() {
	  
	  return getMacParam("appeui");
  }

bool RN2483::setMacParam(char* param, char* value) {
	clearBufferFromModem();
	_stream->print("mac set ");
	_stream->print(param);
	_stream->print(" ");
	_stream->print(value);
	_stream->print("\r\n");
	#ifdef DEBUG
		_debugstream->print("mac set ");
		_debugstream->print(param);
		_debugstream->print(" ");
		_debugstream->print(value);
		_debugstream->print("\r\n");
	#endif
	return expectString("ok");
}

bool RN2483::setRadioParam(char* param, char* value) {
	clearBufferFromModem();
	_stream->print("radio set ");
	_stream->print(param);
	_stream->print(" ");
	_stream->print(value);
	_stream->print("\r\n");
	#ifdef DEBUG
		_debugstream->print("radio set ");
		_debugstream->print(param);
		_debugstream->print(" ");
		_debugstream->print(value);
		_debugstream->print("\r\n");
	#endif
	return expectString("ok");
}

char* RN2483::getMacParam(char* param) {
	clearBufferFromModem();
	_stream->print("mac get ");
	_stream->print(param);
	_stream->print("\r\n");
	delay(100);
	readBufferFromModem();
	return buffer;
}

bool RN2483::saveMac(void) {
	clearBufferFromModem();
	_stream->print("mac save\r\n");
	return expectString("ok", 2000); //this can take a little longer
}

bool RN2483::joinABP() {
	clearBufferFromModem();
	_stream->print("mac join abp\r\n");
	delay(100);
	if(expectString("ok")) {
		delay(1000); //doesn't require a response
		return expectString("accepted");
	}
	else {
		return false;
	}

}

bool RN2483::joinOTAA() {
	clearBufferFromModem();
	_stream->print("mac join otaa\r\n");
	if(expectString("ok")) {
		delay(5000); //this can take a while
		return expectString("accepted");
	}
	else {
		return false;
	}
}

void RN2483::pauseMac(void) {
	clearBufferFromModem();
	_stream->println("mac pause");
}

void RN2483::resumeMac(void) {
	clearBufferFromModem();
	_stream->println("mac resume");
}

bool RN2483::send(byte* payload, uint8_t port, uint8_t len, bool cnf) {
	_stream->print("mac tx ");
	_stream->print(cnf ? "cnf " : "uncnf ");
	_stream->print(port, DEC);
	_stream->print(" ");
	for (uint8_t i = 0; i < len; i++) {
		_stream->print(payload[i] / 0x10, HEX);
		_stream->print(payload[i] % 0x10, HEX);
	}
	_stream->print("\r\n");
	if(expectString("ok")) {
		if(!cnf) {
			delay(100);
			return expectString("mac_tx_ok");
		} else {
			cnfcounter++;
			return true;
		}
	} else {
		return false;
	}
}

void RN2483::handleUART() {
	while(readBufferFromModem() > 0) {
		char receivedCnf[] = "mac_rx 1";
		if(strstr(buffer, receivedCnf) != NULL && cnfcounter > 0) {
			cnfcounter--;
			#ifdef DEBUG
				_debugstream->print("confirmation received, counter: ");
				_debugstream->println(cnfcounter, 10);
			#endif
		}
	}
}

void RN2483::resetCounter() {
	cnfcounter = 0;
}

void RN2483::setLed(bool enabled) {
	clearBufferFromModem();
	_stream->print("sys set pindig GPIO0 ");
	if(enabled) {
		_stream->print("1\r\n");
	} else {
		_stream->print("0\r\n");
	}
	expectString("ok");
}

uint8_t RN2483::readBufferFromModem(unsigned short timeout) {
	buffer[0] = '\0';
	unsigned long threshold = millis() + timeout;
	do{ //optional timeout
		if(_stream->available() > 0) {
			int read = _stream->readBytesUntil('\n', buffer, BUFFERSIZE); //we don't care about CRLF
			buffer[read - 1] = '\0'; //stop string, replace \r by \0
			#ifdef DEBUG
				_debugstream->print(" buffer: ");
				_debugstream->print(buffer);
			#endif
			return (uint8_t)read;
		}
	} while (millis() < threshold || _stream->available() > 0);
	return 0;
}

bool RN2483::expectString(const char* expected, unsigned short timeout) {
	#ifdef DEBUG
		_debugstream->print("expecting: ");
		_debugstream->print(expected);
	#endif
			if(readBufferFromModem(timeout) > 0) {
				if (strstr(buffer, expected) != NULL) {
					#ifdef DEBUG
						_debugstream->println(" ...MATCH");
					#endif
					return true;
				} else {
					#ifdef DEBUG
						_debugstream->println(" ...NO MATCH");
					#endif
					return false;
				}
	} else {
		#ifdef DEBUG
			_debugstream->println("...TIMEOUT");
		#endif
		return false;

	}

}

void RN2483::clearBufferFromModem() {
	while(_stream->available()) {
		_stream->read();
	}
}

