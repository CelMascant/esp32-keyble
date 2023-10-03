#ifndef _KEYPAD_ACCESS_H
#define _KEYPAD_ACCESS_H

#include "Arduino.h"
#include "Wiegand.h"
#include <SPIFFS.h>

#define KEYACC_ArraySize 20
#define KEYACC_EntryLength 10


class KEYPAD_ACCESS {

public:
	KEYPAD_ACCESS();
	void begin(uint8_t D0, uint8_t D1, uint8_t RD, uint8_t GN, uint8_t BUZZ, uint8_t STAR_IN, uint8_t STAR_OUT);
	void begin(uint8_t D0, uint8_t D1);
	bool access_control();
	int lastAccessID();
	int getMode();
	
private:
	String readFile(fs::FS &fs, const char * path);
	void writeFile(fs::FS &fs, const char * path, const char * message);
	void read_access_keys();
	void write_access_keys();
	static unsigned long TagArray[KEYACC_ArraySize];
	static unsigned long PinArray[KEYACC_ArraySize];
	static unsigned long input;
	static String input_String;
	static bool tag_accepted;
	static bool pin_accepted;
	static uint8_t mode;
	static uint8_t pin_number;
	static unsigned long last_code;
	static unsigned long input_timeout;
	static bool b_input_timeout;
	static uint8_t accsess_number;


};

#endif
