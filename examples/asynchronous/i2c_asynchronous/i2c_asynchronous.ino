// WhiteBox Labs -- Tentacle Shield -- I2C asynchronous example
// https://www.whiteboxes.ch/tentacle
//
//
// This code is intended to work on all Arduinos. If using the Arduino Yun, connect
// to it's serial port. If you want to work with the Yun wirelessly, check out the respective
// Yun version of this example.
// It will allow you to control up to 8 Atlas Scientific devices through the I2C bus.
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
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set
// - Change the variables TOTAL_CIRCUITS, channel_ids and channel_names to reflect your setup
// - Set host serial terminal to 9600 baud
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

#include <Wire.h>                              // enable I2C.

#define TOTAL_CIRCUITS 4                       // <-- CHANGE THIS | set how many I2C circuits are attached to the Tentacle

const unsigned int baud_host  = 9600;        // set baud rate for host serial monitor(pc/mac/other)
const unsigned int send_readings_every = 5000; // set at what intervals the readings are sent to the computer (NOTE: this is not the frequency of taking the readings!)
unsigned long next_serial_time;

char sensordata[30];                          // A 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;               // We need to know how many characters bytes have been received
byte code = 0;                                // used to hold the I2C response code.
byte in_char = 0;                             // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.

int channel_ids[] = {97, 98, 99, 100};        // <-- CHANGE THIS. A list of I2C ids that you set your circuits to.
char *channel_names[] = {"DO", "ORP", "PH", "EC"};   // <-- CHANGE THIS. A list of channel names (must be the same order as in channel_ids[]) - only used to designate the readings in serial communications
String readings[TOTAL_CIRCUITS];               // an array of strings to hold the readings of each channel
int channel = 0;                              // INT pointer to hold the current position in the channel_ids/channel_names array

const unsigned int reading_delay = 1000;      // time to wait for the circuit to process a read command. datasheets say 1 second.
unsigned long next_reading_time;              // holds the time when the next reading should be ready from the circuit
boolean request_pending = false;              // wether or not we're waiting for a reading

const unsigned int blink_frequency = 250;     // the frequency of the led blinking, in milliseconds
unsigned long next_blink_time;                // holds the next time the led should change state
boolean led_state = LOW;                      // keeps track of the current led state



void setup() {
  pinMode(13, OUTPUT);                        // set the led output pin
  Serial.begin(baud_host);	              // Set the hardware serial port.
  Wire.begin();			              // enable I2C port.
  next_serial_time = millis() + send_readings_every;  // calculate the next point in time we should do serial communications
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
    for (int i = 0; i < TOTAL_CIRCUITS; i++) {       // loop through all the sensors
      Serial.print(channel_names[i]);                // print channel name
      Serial.print(":\t");
      Serial.println(readings[i]);                     // print the actual reading
    }
    next_serial_time = millis() + send_readings_every;
  }
}


// take sensor readings in a "asynchronous" way
void do_sensor_readings() {
  if (request_pending) {                          // is a request pending?
    if (millis() >= next_reading_time) {          // is it time for the reading to be taken?
      receive_reading();                          // do the actual I2C communication
    }
  } else {                                        // no request is pending,
    channel = (channel + 1) % TOTAL_CIRCUITS;     // switch to the next channel (increase current channel by 1, and roll over if we're at the last channel using the % modulo operator)
    request_reading();                            // do the actual I2C communication
  }
}



// Request a reading from the current channel
void request_reading() {
  request_pending = true;
  Wire.beginTransmission(channel_ids[channel]); // call the circuit by its ID number.
  Wire.write('r');        		        // request a reading by sending 'r'
  Wire.endTransmission();          	        // end the I2C data transmission.
  next_reading_time = millis() + reading_delay; // calculate the next time to request a reading
}



// Receive data from the I2C bus
void receive_reading() {
  sensor_bytes_received = 0;                        // reset data counter
  memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

  Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
  code = Wire.read();

  while (Wire.available()) {          // are there bytes to receive?
    in_char = Wire.read();            // receive a byte.

    if (in_char == 0) {               // if we see that we have been sent a null command.
      Wire.endTransmission();         // end the I2C data transmission.
      break;                          // exit the while loop, we're done here
    }
    else {
      sensordata[sensor_bytes_received] = in_char;  // load this byte into our array.
      sensor_bytes_received++;
    }
  }

  switch (code) {                  	    // switch case based on what the response code is.
    case 1:                       	    // decimal 1  means the command was successful.
      readings[channel] = sensordata;
      break;                        	    // exits the switch case.

    case 2:                        	    // decimal 2 means the command has failed.
      readings[channel] = "error: command failed";
      break;                         	    // exits the switch case.

    case 254:                      	    // decimal 254  means the command has not yet been finished calculating.
      readings[channel] = "reading not ready";
      break;                         	    // exits the switch case.

    case 255:                      	    // decimal 255 means there is no further data to send.
      readings[channel] = "error: no data";
      break;                         	    // exits the switch case.
  }
  request_pending = false;                  // set pending to false, so we can continue to the next sensor
}
