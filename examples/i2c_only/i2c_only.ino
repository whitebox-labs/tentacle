// WhiteBox Labs -- Tentacle Shield -- I2C example
// www.whiteboxes.ch
//
// How to retrieve continuous sensr readings from op to 8 Atlas Scientific devices on the I2C bus
// and send the readings to a host computer via serial.
//
// This code is intended to work on all 5V tolerant Arduinos. If using the Arduino Yun, connect
// to it's usb serial port. If you want to work with the Yun wirelessly, check out the respective
// Yun version of this example.
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set
// - Adjust the variables below to resemble your setup: TOTAL_CIRCUITS, channel_ids, channel_names
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

#include <Wire.h>                     // enable I2C.

char sensordata[30];                  // A 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;       // We need to know how many characters bytes have been received

byte code = 0;                        // used to hold the I2C response code.
byte in_char = 0;                     // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.

#define TOTAL_CIRCUITS 4              // <-- CHANGE THIS | set how many I2C circuits are attached to the Tentacle shield(s): 1-8

int channel_ids[] = {97, 98, 99, 100};// <-- CHANGE THIS.
// A list of I2C ids that you set your circuits to.
// This array should have 1-8 elements (1-8 circuits connected)

char *channel_names[] = {"DO", "ORP", "PH", "EC"}; // <-- CHANGE THIS.
// A list of channel names (must be the same order as in channel_ids[]) 
// it's used to give a name to each sensor ID. This array should have 1-8 elements (1-8 circuits connected).
// {"PH Tank 1", "PH Tank 2", "EC Tank 1", "EC Tank2"}, or {"PH"}



void setup() {                      // startup function
  Serial.begin(9600);	            // Set the hardware serial port.
  Wire.begin();			    // enable I2C port.
}



void loop() {

  for (int channel = 0; channel < TOTAL_CIRCUITS; channel++) {       // loop through all the sensors
  
    Wire.beginTransmission(channel_ids[channel]);     // call the circuit by its ID number.
    Wire.write('r');        		              // request a reading by sending 'r'
    Wire.endTransmission();          	              // end the I2C data transmission.
    
    delay(1000);  // AS circuits need a 1 second before the reading is ready

    sensor_bytes_received = 0;                        // reset data counter
    memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

    Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
    code = Wire.read();

    while (Wire.available()) {          // are there bytes to receive?
      in_char = Wire.read();            // receive a byte.

      if (in_char == 0) {               // null character indicates end of command
        Wire.endTransmission();         // end the I2C data transmission.
        break;                          // exit the while loop, we're done here
      }
      else {
        sensordata[sensor_bytes_received] = in_char;      // append this byte to the sensor data array.
        sensor_bytes_received++;
      }
    }
    
    Serial.print(channel_names[channel]);   // print channel name
    Serial.print(':');

    switch (code) {                  	    // switch case based on what the response code is.
      case 1:                       	    // decimal 1  means the command was successful.
        Serial.println(sensordata);       // print the actual reading
        break;                        	    // exits the switch case.

      case 2:                        	    // decimal 2 means the command has failed.
        Serial.println("command failed");   // print the error
        break;                         	    // exits the switch case.

      case 254:                      	    // decimal 254  means the command has not yet been finished calculating.
        Serial.println("circuit not ready"); // print the error
        break;                         	    // exits the switch case.

      case 255:                      	    // decimal 255 means there is no further data to send.
        Serial.println("no data");          // print the error
        break;                         	    // exits the switch case.
    }

  } // for loop 

}
