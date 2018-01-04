#include <Arduino.h>
#include <SoftwareSerial.h>

#define SERIAL_DELAY 5000
#define SERIAL_RX 1
#define SERIAL_TX 0
#define SERIAL_BAUDRATE 9600
#define SERIAL_RANGE 70


#define PIN_INT_A 3
#define PIN_INT_B 4


#define FI_STATE_FINDING 0
#define FI_STATE_CHECK 1

#define FI_PHASE_OSCCAL_CHANGE 0
#define FI_PHASE_OSCCAL_CHANGE_TIMEOUT 50
#define FI_PHASE_OSCCAL_WAIT 1
#define FI_PHASE_SERIAL_TRY 2
#define FI_PHASE_SERIAL_WAIT 3
#define FI_PHASE_SERIAL_WAIT_TIMEOUT 200

SoftwareSerial tinySerial(SERIAL_RX, SERIAL_TX); // RX, TX

/* ---------------------------------------------
 * @ USART VARIABLE
 * -------------------------------------------*/
char FI_USART_BUFFER[64] = {0};              // a string to hold incoming data
bool FI_USART_RECEIVER_FLAG = false;   // whether the string is complete
int FI_OSCCAL_FACTORY = 0;
int FI_OSCCAL_I = 0;
int FI_OSCCALS[40] = {0};
uint8_t FI_OSCCAL_INDEX = 0;
uint8_t FI_PHASE = 0;
uint8_t FI_STATE = 0;
int16_t FI_J = 0;
uint32_t FI_MS = 0;
void tinySerialEvent(void);

void setup() {
  // put your setup code here, to run once:
  FI_OSCCAL_FACTORY = OSCCAL;
  OSCCAL += -1 * SERIAL_RANGE;
  FI_OSCCAL_I = -1 * SERIAL_RANGE;


  pinMode(PIN_INT_A, OUTPUT); digitalWrite(PIN_INT_A, LOW);
  pinMode(PIN_INT_B, OUTPUT); digitalWrite(PIN_INT_B, LOW);

  // Start the Serial
  tinySerial.begin(SERIAL_BAUDRATE);
  delay(SERIAL_DELAY);
  // INIT
  FI_MS = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(FI_PHASE == FI_PHASE_OSCCAL_CHANGE){
    if(FI_STATE == FI_STATE_FINDING){
      OSCCAL += 1;
      FI_OSCCAL_I += 1;
    }else if(FI_STATE == FI_STATE_CHECK){
      while(!FI_OSCCALS[FI_OSCCAL_INDEX]){
        if(FI_OSCCAL_INDEX < 40 - 1){
          FI_OSCCAL_INDEX++;
        }else{
          digitalWrite(PIN_INT_A, HIGH);
          digitalWrite(PIN_INT_B, HIGH);
          while(1); // DONE 8)
        }
      }
      FI_OSCCAL_I = FI_OSCCALS[FI_OSCCAL_INDEX];
      OSCCAL = FI_OSCCAL_FACTORY + FI_OSCCAL_I;
    }
    FI_PHASE++;
    FI_MS = millis();
  }
  if(FI_PHASE == FI_PHASE_OSCCAL_WAIT){
    if(millis() - FI_MS > FI_PHASE_OSCCAL_CHANGE_TIMEOUT){
      FI_PHASE++;
    }
  }
  if(FI_PHASE == FI_PHASE_SERIAL_TRY){
    digitalWrite(PIN_INT_A, HIGH);
    digitalWrite(PIN_INT_B, LOW);
    tinySerial.println();
    tinySerial.println("3913;ABCDEFGHIJKLMNOPQRSTUVWXYZ;1;");
    //tinySerial.println("3913;abcdefghijklmnopqrstuvwxyz;1;");
    //tinySerial.println("3913;0123456789;1;");
    FI_PHASE++;
    FI_MS = millis();
  }
  if(FI_PHASE == FI_PHASE_SERIAL_WAIT){
    if(millis() - FI_MS > FI_PHASE_SERIAL_WAIT_TIMEOUT){
      if(FI_OSCCAL_I < SERIAL_RANGE && FI_STATE == FI_STATE_FINDING){
        FI_STATE = FI_STATE_FINDING;
      }else if(FI_STATE == FI_STATE_FINDING){
        FI_STATE = FI_STATE_CHECK;
        FI_OSCCAL_INDEX = 0;

        digitalWrite(PIN_INT_A, HIGH);
        digitalWrite(PIN_INT_B, HIGH);
        delay(5000);
      }else if(FI_STATE == FI_STATE_CHECK){
        FI_OSCCAL_INDEX++;
      }
      digitalWrite(PIN_INT_A, LOW);
      FI_PHASE = 0;
    }
  }

  // IF THERE IS A MIRACLE (REPLY)
  if (FI_USART_RECEIVER_FLAG) {
    FI_USART_RECEIVER_FLAG = false;
    char * item = strtok (FI_USART_BUFFER, ";"); //getting first word (uses space & comma as delimeter)
    item = strtok (NULL, ";"); //getting subsequence word
    if(!(strcmp(item,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"))){
      if(FI_STATE == FI_STATE_FINDING){
        FI_OSCCALS[FI_OSCCAL_INDEX] = FI_OSCCAL_I;
        FI_OSCCAL_INDEX++;
        tinySerial.print("3913;OSCCAL;");
        tinySerial.print(FI_OSCCAL_I);
        tinySerial.println(";0;0;"); // add to check list
      }else if(FI_STATE == FI_STATE_CHECK){
        tinySerial.print("3913;OSCCAL;");
        tinySerial.print(FI_OSCCAL_I);
        tinySerial.print(";");
        tinySerial.print(FI_OSCCAL_FACTORY + FI_OSCCAL_I);
        tinySerial.println(";1;");
      }
      digitalWrite(PIN_INT_B, HIGH);
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
