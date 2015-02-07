/*
Arduino code for Linn Benton Community College's Rocksat-C 2015 experiment.
Author: Levi Willmeth
*/
#define buffer_max 50  // Max size of each buffer
#define time_max 60000  // Max runtime

// Geiger tube input pins
#define SHUTDOWN1 = 6
#define SHUTDOWN2 = 7
#define SHUTDOWN3 = 8
#define SHUTDOWN4 = 9

#define C_GATE 2

typedef struct gdata {
  /*
  Each reading has a 4 byte timestamp, and 1 byte of data
  Variables used by interrupts should be called volatile
  */
  volatile unsigned long time;// Timestamp, limited to 71! minutes
  volatile byte tubes;        // 1 byte holds all 6, bit-sized measurements
};

gdata buffer[2][buffer_max+1];// 2 buffers, with space for many readings each
boolean active_buffer = 0;        // determines which buffer is ready for input
byte buffer_index = 0;        // Counter to determine when the buffer is full

void setup(){
  // Debugging only, using serial is very slow.
  Serial.begin(115200);
  Serial.println(F("LBCC RockSat-C code warming up..."));
  
  digitalWrite(SHUTDOWN1, LOW);
  digitalWrite(SHUTDOWN2, HIGH);
  digitalWrite(SHUTDOWN3, HIGH);
  digitalWrite(SHUTDOWN4, LOW);
  
  // Prepare input pins and begin interrupts
  // Pin num, function name, event to listen for
  pinMode(C_GATE, INPUT);
  attachInterrupt(C_GATE, readTubes, RISING);
}

void loop(){
  /* The interrupt will take over if an event occurs,
  so just wait until shutdown.
  
  I'm not certain that using an interrupt is any better
  than just using an if statement right here.?. 
  */
  while(millis() > time_max){
    digitalWrite(SHUTDOWN2, HIGH);
    digitalWrite(SHUTDOWN1, LOW);
    digitalWrite(SHUTDOWN1, LOW);
    digitalWrite(SHUTDOWN3, HIGH);
  }
}

void write_to_sd(){
  /*
  Takes no parameters, writes active buffer to SD card and toggles from
  active to inactive buffer.
  */
  active_buffer = !active_buffer; // Switch between buffers
  /*
  All of this code is for debugging only, and will be replaced by a
  write to the SD card.
  */
  Serial.print(F("\nWriting buffer "));
  // !active_buffer refers to the inactive buffer
  Serial.print(!active_buffer); 
  Serial.print(F(" to the SD card.\n"));
  Serial.println(F("First 10 lines of buffer:"));
  for(int i=0; i<buffer_max; i++){
    Serial.print(buffer[!active_buffer][i].time);
    Serial.print(F("\t"));
    Serial.println(buffer[!active_buffer][i].tubes);
  }
}

void readTubes(){
  /*
  Takes no parameters, stores time and state of each tube into the buffer.
  */
  buffer[active_buffer][buffer_index].time = micros();
  // PINC returns the state of all pins on port C, A0 to A5
  buffer[active_buffer][buffer_index].tubes = PINC;
  // If buffer is full, write results to SD
  if(buffer_index++ == buffer_max){
    write_to_sd();
  }
}
