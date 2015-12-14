// WhiteBox Labs -- Tentacle Shield -- UART asynchronous example -- YUN only
// https://www.whiteboxes.ch/tentacle
//
//
// This sample code was written on an Arduino YUN, and depends on it's Bridge library to
// communicate wirelessly. For Arduino Mega, Uno etc, see the respective examples.
// It will allow you to control up to 8 Atlas Scientific devices through the UART bus.
//
// This example shows how to take readings from the sensors in an asynchronous way, completely
// without using any delays. This allows to do other things while waiting for the sensor data.
// To demonstrate the behaviour, we will blink a simple led in parallel to taking the readings.
// Notice how the led blinks at the desired frequency, not disturbed by the other tasks the
// Arduino is doing.
// 
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to UART, 38400 baud before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Non-EZO (legacy) circuits are supported
// - Change the variables NUM_CIRCUITS, channel_ids and channel_names to reflect your setup
// - To talk to the Yun console, select your Yun's name and IP address in the Port menu.
//    - The Yun will only show up in the Ports menu, if your computer is on the same Network as the Yun.
//
//---------------------------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------------------------

#include <Wire.h>                   // enable I2C.
#include <Console.h>                // Yun Console
#include <SoftwareSerial.h>         //Include the software serial library  
SoftwareSerial sSerial(11, 10);     // RX, TX  - Name the software serial library sftSerial (this cannot be omitted)
                                    // assigned to pins 10 and 11 for maximum compatibility
                                    
#define NUM_CIRCUITS 4              // <-- CHANGE THIS | set how many UART circuits are attached to the Tentacle

const unsigned int send_readings_every = 5000; // set at what intervals the readings are sent to the computer (NOTE: this is not the frequency of taking the readings!)
unsigned long next_serial_time;

#define baud_circuits 38400         // NOTE: older circuit versions have a fixed baudrate (e.g. 38400. pick a baudrate that all your circuits understand and configure them accordingly)
int s0 = 7;                         // Tentacle uses pin 7 for multiplexer control S0
int s1 = 6;                         // Tentacle uses pin 6 for multiplexer control S1
int enable_1 = 5;	            // Tentacle uses pin 5 to control pin E on shield 1
int enable_2 = 4;	            // Tentacle uses pin 4 to control pin E on shield 2
char sensordata[30];                          // A 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;               // We need to know how many characters bytes have been received
byte code = 0;                                // used to hold the I2C response code.
byte in_char = 0;                             // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.

char *channel_names[] = {"DO", "ORP", "PH", "EC"};   // <-- CHANGE THIS. A list of channel names (this list should have TOTAL_CIRCUITS entries)
                                                     // only used to designate the readings in serial communications
                                                     // position in array defines the serial channel. e.g. channel_names[0] is channel 0 on the shield; "PH" in this case.
String readings[NUM_CIRCUITS];                // an array of strings to hold the readings of each channel
int channel = 0;                              // INT pointer to hold the current position in the channel_ids/channel_names array

const unsigned int reading_delay = 100;       // how often to poll the soft serial bus for new data.
                                              // low values give fast reading updates, <1 sec per circuit.
                                              // high values give your Ardino more time for other stuff
unsigned long next_reading_time;              // holds the time when the next reading should be ready from the circuit
boolean request_pending = false;              // wether or not we're waiting for a reading

const unsigned int blink_frequency = 250;     // the frequency of the led blinking, in milliseconds
unsigned long next_blink_time;                // holds the next time the led should change state
boolean led_state = LOW;                      // keeps track of the current led state



void setup() {
  pinMode(13, OUTPUT);                        // set the led output pin
  
  pinMode(s0, OUTPUT);                        // set the digital output pins for the serial multiplexer
  pinMode(s1, OUTPUT);
  pinMode(enable_1, OUTPUT);
  pinMode(enable_2, OUTPUT);

  Bridge.begin();
  Console.begin();                            // initialize serial communication over network:
  while (!Console) ;                          // wait for Console port to connect.
  Wire.begin();			              // enable I2C port.
  sSerial.begin(baud_circuits);               // Set the soft serial port to 9600 (change if all your devices use another baudrate)
  next_serial_time = millis() + send_readings_every;  // calculate the next point in time we should do serial communications
  Console.println("-----");
}



void loop() {
  do_sensor_readings();
  do_serial();

  // Do other stuff here (Blink Leds, update a display, etc)
  
  blink_led();
}



// blinks a led on pin 13 asynchronously 
void blink_led() {
  if (millis() >= next_blink_time) {                  // is it time for the blink already?
    led_state = !led_state;                           // toggle led state on/off
    digitalWrite(13, led_state);                      // write the led state to the led pin
    next_blink_time = millis() + blink_frequency;     // calculate the next time a blink is due
  }
}



// do serial communication in a "asynchronous" way
void do_serial() {
  if (millis() >= next_serial_time) {                // is it time for the next serial communication?
    Console.println("-------------");
    for (int i = 0; i < NUM_CIRCUITS; i++) {         // loop through all the sensors
      Console.print(channel_names[i]);                // print channel name
      Console.print(":\t");
      Console.println(readings[i]);                   // print the actual reading
    }
    Console.println("-");
    next_serial_time = millis() + send_readings_every;
  }
}


// take sensor readings in a "asynchronous" way
void do_sensor_readings() {
  
  if (request_pending) {                          // is a request pending?

    if (millis()>next_reading_time) {
      if (sSerial.available()) {                  // If data has been transmitted from an Atlas Scientific device
    
        sensor_bytes_received = sSerial.readBytesUntil(13, sensordata, 30);  // read until we see a <cr> char
        sensordata[sensor_bytes_received] = 0; 
        readings[channel] = sensordata;
  
        request_pending = false;                            
        sensor_bytes_received = 0;                            // reset data counter
        memset(sensordata, 0, sizeof(sensordata));            // clear sensordata array;
        
        // un-comment to see the real update frequency of the readings / debug
        //Console.print(channel_names[channel]);
        //Console.print(" update:\t");
        //Console.println(readings[channel]);
    
      } else {
        next_reading_time = millis()+reading_delay; 
      }
    }

  } else {                                     // no request is pending,
    switch_channel();
    request_reading();                         // do the actual I2C communication
  }
}


void switch_channel() {
  channel = (channel + 1) % NUM_CIRCUITS;     // switch to the next channel (increase current channel by 1, and roll over if we're at the last channel using the % modulo operator) 
  open_channel();                             // configure the multiplexer for the new channel - we "hot swap" the circuit connected to the softSerial pins
  while (sSerial.available()) {               // clear out everything that is in the buffer already
   sSerial.read(); 
  }
}



// Request a reading from the current channel
void request_reading() {
    request_pending = true;
    sSerial.print("r\r");                     // <CR> carriage return to terminate message
}


// Open a channel via the Tentacle serial multiplexer
void open_channel() {

  switch (channel) {

    case 0:                                  // if channel==0 then we open channel 0
      digitalWrite(enable_1, LOW);           // setting enable_1 to low activates primary channels: 0,1,2,3
      digitalWrite(enable_2, HIGH);          // setting enable_2 to high deactivates secondary channels: 4,5,6,7
      digitalWrite(s0, LOW);                 // S0 and S1 control what channel opens
      digitalWrite(s1, LOW);                 // S0 and S1 control what channel opens
      break;

    case 1:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      break;

    case 2:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      break;

    case 3:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      break;

    case 4:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, LOW);
      break;

    case 5:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      break;

    case '6':
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      break;

    case 7:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      break;
  }
}
