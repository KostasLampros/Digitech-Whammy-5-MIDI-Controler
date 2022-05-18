#include <Arduino.h>
#include <MIDI.h>
#include <EEPROM.h>

//board = pro16MHzatmega328 or uno


#define DEBUG 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define midi(x) 
#else
#define debug(x)
#define debugln(x)
#define midi(x) MIDI.begin(x)
#endif

MIDI_CREATE_DEFAULT_INSTANCE(); 

//Preference variables
size_t tap_delay = 200; // Delay before next change registration
int duration = 400; // Duration needed to change rotation
const int buttonPin = 2; //Stomp switch pin
const int switch_ = 6; //Switch to change modes: playng/updating
const int led_norm = 5; // Led for normal looping mode
const int led_rev = 4; // Led for reversed looping mode
const int saveButton = 3; //Button to save a programm in memory
//const int midipower = 7; //midi 5V power instead of VCC for convenience

//Static variables
unsigned long blinkTimer = millis();
unsigned long startPressed = 0; //initial   // The moment the button was pressed
unsigned long last_call = -1000; //Last time the buuton was pressed
unsigned long last_call_save = -1000; // Last time the save buton was peressed
int press_time = 0;   // press_time     // how long the button was hold
bool rev = false; // Looping mode
int eeSize = 0; //Number of programms in memory
int prog = 0; //pogramm curently in update mode
int poss; //possition of existing programm
bool in = false; //indicator if program is in memmory
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 30;    // the debounce time; increase if the output flickers

int reading = 1;           // state of the button without debouncing
int buttonState = 1;       // current state of the button
int lastButtonState = 1;   // previous state of the button
int i = -1;
int j = 1;
int k;
int q = -1;
int w = 1;

//Function to loop between SAVED programs
void changeProg() {
  // the button has been just pressed
  if (buttonState == LOW) {
      startPressed = millis();
  // the button has been just released
  } else {
      if(millis()-last_call<tap_delay){  // Check from last press to avoid double tap
            return;
      }else{
        last_call = millis();
        press_time = last_call - startPressed;
  
        if (press_time >= duration) {
            if(!rev){  // check if it is in reverse mode or not
              j = eeSize -1;  //Modification of i progression to reverse 
              rev = true;
              digitalWrite(led_rev, HIGH); // turn on reversed led
            }else{
              j = 1;
              rev = false;
              digitalWrite(led_rev, LOW); // turn off reversed led
            }
        }
        i+=j;
        MIDI.sendProgramChange(EEPROM[i%eeSize] + 63,1); //sends variable of programms  in position i  in channel 1
        debugln(String(EEPROM[i%eeSize])+",");
  
      }
  }
}

//Function to loop between ALL programs
void loopProg(){
    // the button has been just pressed
    if (buttonState == LOW) {
        startPressed = millis();
    } else {
      if(millis()-last_call<tap_delay){  // Check from last press to avoid double tap
            debugln("broke");
            return;
      }else{
        last_call = millis();
        press_time = last_call - startPressed;
        
  
        if (press_time >= duration) {
            if(!rev){  // check if it is in reverse mode or not
              w = 21 -1;  //Modification of i progression to reverse 
              rev = true;
              digitalWrite(led_rev, HIGH); // turn on reversed led
              //Serial.println("rev mode");
            }else{
              digitalWrite(led_rev, LOW); // turn on reversed led
              //Serial.println("normal mode");
              w = 1;
              rev = false;
            }
        }
        q+=w;
        MIDI.sendProgramChange((q%21 + 63),1); //sends variable of programms  in position i  in channel 1
        debugln("prog ="+ String(q%21));
        prog = q%21;
        
     }
    
  }

}

//Function to show all the saved programs for debuging
void show(int length){
  for (int k = 0; k < length; k++){
    if( EEPROM.read(k)!=255 && k>0){
        debug(",");
      }
    if(EEPROM.read(k)!=255){
      debug(EEPROM.read(k));
      
      
    }
  }
  debugln("");
}

void setup() 
{ 
  pinMode(buttonPin, INPUT_PULLUP); // initialize the button pin as a input
  Serial.begin(9600);        // initialize serial communication
  pinMode(led_norm, OUTPUT); // initialize the LED pin as a output
  pinMode(led_rev, OUTPUT); // initialize the LED pin as a output
  pinMode(switch_, INPUT_PULLUP); // initialize the switch pin as a input
  pinMode(saveButton, INPUT_PULLUP); // initialize the switch pin as a input
  //pinMode(midipower, OUTPUT);
  midi(MIDI_CHANNEL_OFF); // start midi lib

  //digitalWrite(midipower,HIGH);
  debugln("Starting");
  
  while (EEPROM.read(eeSize)!= 255){ 
    eeSize++;
  }
  show(eeSize);
  debugln(eeSize);
}

void loop(){
  //playing mode
  if(digitalRead(switch_) == HIGH){
    if (i==-1){
      i++;
      MIDI.sendProgramChange(EEPROM[i%eeSize] + 63,1); //sends variable of programms  in position i  in channel 1
      debugln(String(EEPROM[i%eeSize])+",");
      q = -1;
      w = 1;
      digitalWrite(led_norm,LOW); //In case blink() ended with HIGH value
      if( rev == true){
        j = eeSize-1;
      }
    }
  
    reading = digitalRead(buttonPin); // read the button input
    
    if (reading != lastButtonState) { // button state changed
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) { //ignore inputs in debounce range
      if(reading != buttonState){
      buttonState = reading;
      changeProg();
      }
    }

    lastButtonState = reading;        // save state for next loop

        
  //Saving mode
  }else{
    if (q==-1){
      q++;
      MIDI.sendProgramChange((q%21 + 63),1); //sends variable of programms  in position i  in channel 1
      debugln("prog ="+ String(q%21));
      prog = q%21;
      i = -1;
      j = 1;
      digitalWrite(led_norm,LOW); // In case blink() ended with HIGH value
      if( rev == true){
        w = 21-1;
      }
    }
    
    reading = digitalRead(buttonPin); // read the button input
    
    if (reading != lastButtonState) { // button state changed
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) { //ignore inputs in debounce range
      if(reading != buttonState){
      buttonState = reading;
      loopProg();
      }
    }

    lastButtonState = reading;        // save state for next loop
    

    if(prog != -1){ //If prog variable is an actual programm
        for(int k = 0; k<=eeSize; k++){ 
            if( prog == EEPROM[k]){ // Check if programm is already in memory
                in = true;
                poss = k; // Possition of programm in array of memory
                digitalWrite(led_norm,HIGH); // Show it is in memory 
                break;
            }else{
                in = false;
                digitalWrite(led_norm,LOW);
            }
        }
        if(digitalRead(saveButton) == LOW){ //If button to save programm is pressed
          while(digitalRead(saveButton) == LOW){ //Do nothing until it is unpressed
            ;
          }
          if(millis()-last_call_save<tap_delay){
            debugln("save broke");
            return;
          }else{
            last_call_save =millis();
            if (in == true){ // If programm was in memory
                for(poss; poss<=eeSize; poss++){
                    EEPROM[poss] = EEPROM[poss+1]; // Delete it and move all others one stem to the left
                }
                eeSize--; // Decrise size of the array
                debugln("Removed programm " + String(prog));
            }else{ //If programm was not in memory 
                for(k=0; k<eeSize; k++){
                    if(prog <EEPROM[k]){ // Find the place it needs to be placed in ascending order 
                        poss = k;
                        break;
                    }else{
                        poss = k+1;
                    }              
                }

                for(k=eeSize+1; k>=poss;k--){
                    EEPROM[k]=EEPROM[k-1]; //Move array one step to the left and place new programm
                }
                EEPROM[poss] = prog;
                eeSize++; //Increase array size
                debugln("New programm added " + String(prog));    
            }
          }
        }


    }
  }
  
}
