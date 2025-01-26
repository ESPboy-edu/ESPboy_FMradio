/*
   Test and validation of the circuit.
   This sketch will check the I2C communication bus and try to tune the receiver on a given frequency.
 
    Arduino Pro Mini and SI4703 wire up

    | Device  Si470X |  Arduino Pin  |
    | ---------------| ------------  |
    | RESET          |     14/A0     |
    | SDIO           |     A4        |
    | SCLK           |     A5        |


   ATTENTION:  
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer. 
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board.

   The main advantages of using this sketch are: 
    1) It is a easy way to check if your circuit is working;
    2) You do not need to connect any display device to make your radio works;
    3) You do not need connect any push buttons or encoders to change volume and frequency;
    4) The Arduino IDE is all you need to check your circuit.  
   
    By Ricardo Lima Caratti, 2020.
*/

#include <SI470X.h>

#define RESET_PIN 14 // On Arduino Atmega328 based board, this pin is labeled as A0 (14 means digital pin instead analog)

SI470X rx; 

void setup() {

  Serial.begin(9600);
  while(!Serial);

  delay(500);
  

  rx.setup(RESET_PIN, A4);

  if (!checkI2C())
  {
      Serial.println("\nCheck your circuit!");
      while(1);
  }

  Serial.print("\nPN........................:");
  Serial.print(rx.getPartNumber(),HEX);
  Serial.print("\nManufacturer..............:");
  Serial.print(rx.getPartNumber(),HEX);
  Serial.print("\nCHIP Version..............:");
  Serial.print(rx.getChipVersion(),HEX);    
  Serial.print("\nDevice....................:");
  Serial.print(rx.getDeviceId(),BIN);
  Serial.print("\nFirmware..................:");
  Serial.print(rx.getFirmwareVersion(),BIN);

  rx.setVolume(6);  
  
  delay(500);
  //****
  Serial.print("\nEstacao 106.5MHz");
  rx.setFrequency(10650); // The frequency you want to select in MHz multiplied by 100.
  
  Serial.print("\nCurrent Channel: ");
  Serial.print(rx.getRealChannel());
  delay(500);

  Serial.print("\nReal Frequency.: ");
  Serial.print(rx.getRealFrequency());
  
  Serial.print("\nRSSI: ");
  Serial.print(rx.getRssi());

  
  // Mute test
  Serial.print("\nAfter 5s device will mute during 3s");
  delay(5000);
  rx.setMute(true);
  delay(3000);
  rx.setMute(false);
  Serial.print("\nMute test has finished.");

  // Seek test
  Serial.print("\nSeek station");
  for (int i = 0; i < 10; i++ ) { 
    rx.seek(1,0);
    Serial.print("\nReal Frequency.: ");
    Serial.print(rx.getRealFrequency());
    delay(5000);
  }


  Serial.print("\nEstacao 106.5MHz");
  rx.setFrequency(10650);
  
  
}

void loop() {

 

}

/**
 * Returns true if device found
 */
bool checkI2C() {
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("\nI2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("\nUnknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return false;
  }
  else {
    Serial.println("done\n");
    return true;
  }
}
