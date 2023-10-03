#include "Keypad_access_control.h"
#include <SPIFFS.h>

unsigned long KEYPAD_ACCESS::TagArray[KEYACC_ArraySize];
unsigned long KEYPAD_ACCESS::PinArray[KEYACC_ArraySize];

unsigned long KEYPAD_ACCESS::input= 0;
bool KEYPAD_ACCESS::tag_accepted = false;
bool KEYPAD_ACCESS::pin_accepted = false;
uint8_t KEYPAD_ACCESS::mode = 0;
uint8_t KEYPAD_ACCESS::pin_number = 0;

String KEYPAD_ACCESS::input_String;
unsigned long KEYPAD_ACCESS::last_code = 0;
unsigned long KEYPAD_ACCESS::input_timeout = 2000;
bool KEYPAD_ACCESS::b_input_timeout = false;
uint8_t KEYPAD_ACCESS::accsess_number = 0;

WIEGAND wg;

KEYPAD_ACCESS::KEYPAD_ACCESS()
{
}

void KEYPAD_ACCESS::begin(uint8_t D0, uint8_t D1, uint8_t RD, uint8_t GN, uint8_t BUZZ, uint8_t STAR_IN, uint8_t STAR_OUT){
	wg.begin(D0, D1);
	pinMode(RD, OUTPUT);
	pinMode(GN, OUTPUT);
	pinMode(BUZZ, OUTPUT);
	pinMode(STAR_IN, INPUT_PULLUP);
	pinMode(STAR_OUT, OUTPUT);
	read_access_keys();
}

void KEYPAD_ACCESS::begin(uint8_t D0, uint8_t D1){
	wg.begin(D0, D1);
	read_access_keys();
}

String KEYPAD_ACCESS::readFile(fs::FS &fs, const char * path){
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  return fileContent;
}

void KEYPAD_ACCESS::writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
  } else {
    Serial.println("- write failed");
  }
}

void KEYPAD_ACCESS::read_access_keys(){

  // read tags from file
  Serial.println("Read tags from file...");
  String DataString = readFile(SPIFFS, "/tags.txt");
  int x = 0;
  int y = 0;
  char *ptr;
  char DataDelimiter = ';';
  char Data[KEYACC_EntryLength];
  x = DataString.indexOf(DataDelimiter, y);
  TagArray[0] = strtoul(strcpy(Data, DataString.substring(y,x).c_str()), &ptr, 10);
  Serial.print("TagArray[0] = ");
  Serial.println(TagArray[0]);
  for (uint8_t i = 1; i < KEYACC_ArraySize; i++){
    y=x+1; 
    x = DataString.indexOf(DataDelimiter, y); 
    TagArray[i] = strtoul(strcpy(Data, DataString.substring(y,x).c_str()), &ptr, 10);
    Serial.print("TagArray[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(TagArray[i]);
  }  
	Serial.println(" Done.");

  // read pins from file
  Serial.println("Read pins from file...");
  DataString = readFile(SPIFFS, "/pins.txt");
  x = 0;
  y = 0;
  DataDelimiter = ';';
  x = DataString.indexOf(DataDelimiter, y);
  PinArray[0] = strtoul(strcpy(Data, DataString.substring(y,x).c_str()), &ptr, 10);
  Serial.print("Master pin = ");
  Serial.println(PinArray[0]);
  for (uint8_t i = 1; i < KEYACC_ArraySize; i++){
    y=x+1; x = DataString.indexOf(DataDelimiter, y); 
    PinArray[i] = strtoul(strcpy(Data, DataString.substring(y,x).c_str()), &ptr, 10);
    Serial.print("PinArray[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(PinArray[i]);
  }
  Serial.println("done.");
  Serial.println("Key init completed.");
}

void KEYPAD_ACCESS::write_access_keys(){
  String helperString = "";
  char safestring[10*(KEYACC_ArraySize+1)];
  for (uint8_t i = 0; i < KEYACC_ArraySize; i++){
      helperString.concat(TagArray[i]);
      helperString.concat(";");
  }
  helperString.toCharArray(safestring, 10*(KEYACC_ArraySize+1));
  Serial.print("Write tags to SPIFFS: ");
  Serial.println(safestring);
  writeFile(SPIFFS, "/tags.txt", safestring);

  for (uint8_t i = 0; i < KEYACC_ArraySize; i++){
      helperString.concat(PinArray[i]);
      helperString.concat(";");
  }
  helperString.toCharArray(safestring, 10*(KEYACC_ArraySize+1));
  Serial.print("Write pins to SPIFFS: ");
  Serial.println(safestring);
  writeFile(SPIFFS, "/pins.txt", safestring);
}

bool KEYPAD_ACCESS::access_control(){
	
bool ret = false;
if(wg.available()) // tag was read or button was pressed
	{
    input = wg.getCode();
    accsess_number = 0;
    last_code = millis();
		Serial.print("Wiegand Input dez:");
		Serial.println(input);
    tag_accepted = false;
    pin_accepted = false;
    if(PinArray[0] == 0){
		Serial.println("No Master pin. Switch to mode 255!");
      mode = 255;
    }
	Serial.print("Keypad Mode: ");
	Serial.println(mode);
    switch (mode){
      case 0: // normal operation mode
        if(input > 255){ //bigger numbers are no ascii-characters -> tag-ID
          Serial.print("tag detected. ID: ");
          Serial.println(input);
          for (accsess_number = 0; accsess_number < KEYACC_ArraySize; accsess_number++){
            if (input == TagArray[accsess_number]){
			        ret = true;
              tag_accepted = true;
              Serial.print("Acsess granted. Tag number: ");
              Serial.println(accsess_number);
            }
          }
          if (tag_accepted == false){
            //do something when an unautorized tag was presented
			      ret = false;
            Serial.print("Access denied. Tag ID: ");
            Serial.println(input);
          }

        } else if (input == 13) { // button # was pressed -> End of code input. Check if entered code is valid. 
          Serial.print("pin entered. Pin: ");
          Serial.println(input_String.toInt());
          if(input_String.toInt() >= 999){   
            for (accsess_number = 0; accsess_number < KEYACC_ArraySize; accsess_number++){ //check the array if one entry match the input
              if (input_String.toInt() == PinArray[accsess_number]){
                pin_accepted = true;
                if(accsess_number == 0){
                  mode = 1;
                  ret = false;
                  Serial.println("Master pin entered.");
                } else {
                  ret = true;
                  Serial.print("Acsess granted. PIN: ");
                  Serial.println(accsess_number, DEC);
                }
              }
            }
          }
          if (pin_accepted == false){
            //do something when an unautorized pin was entered
			      ret = false;
            Serial.print("Acsess denied. Wrong pin:");
            Serial.println(input_String);
          }
          input_String = "0";
        } else { // Button was pressed. add it to the button string
          input_String.concat(input);
        }
        break;

        case 1: // programming menu - chose function: 1=new RFID tag, 2=new pin code, every other number -> abort
          if(input == 13){ 
            if(input_String.toInt() == 1){
              mode = 11;
            }else if(input_String.toInt() == 2){
              mode = 21;
            } else {
              mode = 0;
            }
            Serial.print("Set mode to ");
            Serial.println(mode);
            input_String = "0";
          } else {// Button was pressed. add it to the button string
            input_String.concat(input);
          }
        break;

        case 11: // programming menu - new RFID tag: enter tag id
          if(input == 13){ 
            if(input_String.toInt() <= KEYACC_ArraySize){
              pin_number = input_String.toInt();
              Serial.print("Set tag nr. ");
              Serial.println(pin_number);
              mode = 12;
            } else {
              mode = 0;
            }
            input_String = "0";
          } else {// Button was pressed. add it to the button string
            input_String.concat(input);
          }

        break;

        case 12: // programming menu - new RFID tag: read tag
          if(input > 255){
              TagArray[pin_number] = input;
              Serial.print("Set tag nr. ");
              Serial.print(pin_number);
              Serial.print(" to ");
              Serial.println(TagArray[pin_number]);
              write_access_keys();
              mode = 0;
            } else if (input == 13){
              TagArray[pin_number] = 0;
              Serial.print("Delete tag nr. ");
              Serial.print(pin_number);
              write_access_keys();
              mode = 0;
            }
            
        break;

        case 21: // programming menu - new pin code: enter pin id
          if(input == 13){
            if(input_String.toInt() <= KEYACC_ArraySize){
              pin_number = input_String.toInt();
              Serial.print("Set pin nr. ");
              Serial.println(pin_number);
              mode = 22;
            } else {
              mode = 0;
            }
            input_String = "0";
          } else {// Button was pressed. add it to the button string
            input_String.concat(input);
          }
        break;

        case 22: // programming menu - new pin code: enter pin code
          if(input == 13){ 
            if((input_String.toInt() >= 999 && input_String.toInt() <= 4294967295)||input_String.toInt() == 0){
              PinArray[pin_number] = input_String.toInt();
              Serial.print("Set pin nr. ");
              Serial.print(pin_number);
              Serial.print(" to ");
              Serial.println(PinArray[pin_number]);
              write_access_keys();
            }
            mode = 0;
            input_String = "0";
          } else {// Button was pressed. add it to the button string
            input_String.concat(input);
          }
        break;

        case 255: // No Master pin! Enter Master pin!
          if(input == 13){ 
            if(sizeof(input_String) >= 4 && sizeof(input_String) <= 8){
			        PinArray[0] = input_String.toInt();
              Serial.print("Set new master pin: ");
              Serial.println(PinArray[0]);
              mode = 0;
              write_access_keys();
            }
            input_String = "0";
          } else {// Button was pressed. add it to the button string
            input_String.concat(input);
          }
        break;

        default:
          mode = 0;
        break;
      }
   } else {
	ret = false;
   }

   if (millis() > last_code + input_timeout) {
    ret = false;
   }
   return(ret);
}

int KEYPAD_ACCESS::lastAccessID(){
	return(0);
};

int KEYPAD_ACCESS::getMode(){
	return(0);
};