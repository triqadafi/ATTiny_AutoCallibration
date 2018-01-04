#include <Arduino.h>
#include <SoftwareSerial.h>

#define SERIAL_RX 1
#define SERIAL_TX 0
#define SERIAL_BAUDRATE 9600
#define SERIAL_RANGE 70

SoftwareSerial tinySerial(SERIAL_RX, SERIAL_TX); // RX, TX

/* ---------------------------------------------
 * @ USART VARIABLE
 * -------------------------------------------*/
char FI_USART_BUFFER[64] = {0};              // a string to hold incoming data
bool FI_USART_RECEIVER_FLAG = false;   // whether the string is complete
int FACTORY_OSCCAL = 0;
int FI_OSCCAL[20] = {0};
uint8_t FI_OSCCAL_INDEX = 0;
uint8_t FI_PHASE = 0;
int16_t FI_I = 0;
int16_t FI_J = 0;
uint32_t FI_MS = 0;
void tinySerialEvent(void);

void setup() {
  // put your setup code here, to run once:
  //FACTORY_OSCCAL = OSCCAL;
  OSCCAL -= SERIAL_RANGE;
  FI_I = -1 * SERIAL_RANGE;
  // Start the Serial
  tinySerial.begin(SERIAL_BAUDRATE);
  delay(5000);
  // INIT
  FI_MS = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(FI_PHASE == 0){
    if(FI_I < SERIAL_RANGE){
      OSCCAL += 1;
      FI_I += 1;
      FI_MS = millis();
      FI_PHASE++;
    }else{
      if(FI_OSCCAL[FI_J] != 0){
        FI_I = FACTORY_OSCCAL + FI_OSCCAL[FI_J];
        OSCCAL = FI_I;
        FI_J += 1;
        FI_MS = millis();
        FI_PHASE++;
      }
    }
  }
  if(FI_PHASE == 1){
    if(millis() - FI_MS > 500){
      FI_PHASE++;
    }
  }
  if(FI_PHASE == 2){
    tinySerial.println();
    tinySerial.print("3913;OSCCAL +=");
    tinySerial.print(abs(FI_I));
    tinySerial.print(";");
    tinySerial.print(FACTORY_OSCCAL);
    tinySerial.println(";");
    tinySerial.println("3913;ABCDEFGHIJKLMNOPQRSTUVWXYZ;");
    FI_MS = millis();
    FI_PHASE++;
  }
  if(FI_PHASE == 3){
    if(millis() - FI_MS > 1000){
      FI_PHASE = 0;
    }
  }
  if (FI_USART_RECEIVER_FLAG) {
    FI_USART_RECEIVER_FLAG = false;
    char * item = strtok (FI_USART_BUFFER, ";"); //getting first word (uses space & comma as delimeter)
    item = strtok (NULL, ";"); //getting subsequence word
    if(!(strcmp(item,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"))){
      FI_OSCCAL[FI_OSCCAL_INDEX] = FI_I;
      FI_OSCCAL_INDEX++;
      tinySerial.println("3913;OK;1;");
    }
  }

  tinySerialEvent();
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
uint8_t FI_USART_BUFFER_INDEX = 0;
void tinySerialEvent() {
  while (tinySerial.available()) {
    // get the new byte:
    char inChar = (char)tinySerial.read();
    // add it to the inputString:
    FI_USART_BUFFER[FI_USART_BUFFER_INDEX] = inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      FI_USART_BUFFER[FI_USART_BUFFER_INDEX+1] = '\0';
      FI_USART_BUFFER_INDEX=0;
      if(strncmp(FI_USART_BUFFER, "3913", 4) == 0) FI_USART_RECEIVER_FLAG = true;
    }else{
      FI_USART_BUFFER_INDEX ++;
    }
  }
}
