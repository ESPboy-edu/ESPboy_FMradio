/*
   Test and validation of SI4703 on STM32 board.
    
   ATTENTION:  
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer. 
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board. 
    
    | Si470x pin      |     STM32     |
    | ----------------| ------------  |
    | RESET /RST      |     PA12      |
    | SDIO / SDA      |     PB7 (B7)  |
    | SCLK / CLK      |     PB6 (B6)  |

   By Ricardo Lima Caratti, 2020.
*/

#include <SI470X.h>

#define RESET_PIN PA12       // On Arduino Atmega328 based board, this pin is labeled as A0 (14 means digital pin instead analog)
#define STM32_SDA_PIN   PB7        //  

#define MAX_DELAY_RDS 40   // 40ms - polling method

long rds_elapsed = millis();

SI470X rx;


void showHelp()
{
  /*
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(1000);*/
}


// Show current frequency
void showStatus()
{
  char aux[80];
  sprintf(aux,"\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | %s ",rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No" );
  // Serial.print(aux);
}


void setup()
{
    // Serial.begin(115200);
    // while (!Serial) ;
    
    rx.setup(RESET_PIN, STM32_SDA_PIN);

    rx.setVolume(6);

    delay(500);

    // Select a station with RDS service in your place
    // Serial.print("\nEstacao 106.5MHz");
    rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.

    // Enables SDR
    rx.setRds(true);
    rx.setRdsMode(0); 
    rx.setSeekThreshold(30); // Sets RSSI Seek Threshold (0 to 127)

    showHelp();
    showStatus();
}

void loop()
{
  /*
  if (Serial.available() > 0)
  {
    char key = Serial.read();
    switch (key)
    {
    case '+':
      rx.setVolumeUp();
      break;
    case '-':
      rx.setVolumeDown();
      break;
    case 'U':
    case 'u':
      rx.setFrequencyUp();
      break;
    case 'D':
    case 'd':
      rx.setFrequencyDown();
      break;
    case 'S':
      rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_UP);
      break;
    case 's':
      rx.seek(SI470X_SEEK_WRAP, SI470X_SEEK_DOWN);
      break;
    case '0':
      showStatus();
      break;
    case '?':
      showHelp();
      break;
    default:
      break;
    }
    delay(200);
    showStatus();
  } */
  delay(5);
}
