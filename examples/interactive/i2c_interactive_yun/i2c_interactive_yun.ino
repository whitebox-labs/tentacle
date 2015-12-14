// WhiteBox Labs -- Tentacle Shield -- I2C interactive example
// https://www.whiteboxes.ch/tentacle
//
// How to use 4 (or 8 if using 2 Tentacle shields) Atlas Scientivic devices in I2C mode
// and interact with them via the serial monitor.
//
// This sample code was written on an Arduino YUN, and depends on it's Bridge library to
// communicate wirelessly. For Arduino Mega, Uno etc, see the respective examples.
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set 
// - Set host serial terminal to 9600 baud
//
// - To send a command, send the number of the i2c address, a colon and the command ending with a carriage return.
//
// - To issue a command, enter it directly to the console.
//
// 102:r<CR>
// 99:i<CR>
// 100:c<CR>
// 99:r<CR>
// 110:cal,mid,7.00<CR>
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

unsigned long serial_host  = 9600;  // set baud rate for host serial monitor(pc/mac/other)

char sensordata[30];                // A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received = 0;   // We need to know how many characters bytes have been received
int computer_in_byte;               // a variable to read incoming console data into
boolean computer_msg_complete = false;
byte sensor_bytes_received = 0;     // We need to know how many characters bytes have been received
int channel;                        // INT pointer for channel switching - 0-7 serial, 8-127 I2C addresses
char *cmd;                          // Char pointer used in string parsing

char computerdata[48];              // we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte code = 0;                      // used to hold the I2C response code.
byte in_char = 0;                   // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.
int time;                   	    // used to change the dynamic polling delay needed for I2C read operations.


void setup() {                      // startup function
  Bridge.begin();
  Console.begin();                  // initialize serial communication over network:
  while (!Console) ;                // wait for Console port to connect.
  Wire.begin();			    // enable I2C port.
  intro();			    // display startup message
}



void loop() {                                 	// main loop

 while (Console.available() > 0) {             // On Yun, there's no serialEvent(), so we read all data from the console here
    computer_in_byte = Console.read();          // read a byte

    if (computer_in_byte == '\n' || computer_in_byte == '\r') {      // if a newline character arrives, we assume a complete command has been received
      computerdata[computer_bytes_received] = 0;
      computer_msg_complete = true;
      computer_bytes_received = 0;
    } else {                                    // add the new byte to the computerdata string
      computerdata[computer_bytes_received] = computer_in_byte;
      computer_bytes_received++;
    }
  }

  if (computer_msg_complete) {                  // if we received a command from the computer
    channel = atoi(strtok(computerdata, ":"));        // let's parse the string at each colon
    cmd = strtok(NULL, ":");
    if (cmd != 0) {                             // if no command has been sent, send nothing
      I2C_call();                               // make the actual I2C communication
    }
    computer_msg_complete = false;              // reset the var computer_msg_complete to be ready for the next command
  }

}

void intro() {                                  // print intro
  Console.flush();
  Console.println(" ");
  Console.println("READY_");
}


void I2C_call() {  			        // function to parse and call I2C commands
  sensor_bytes_received = 0;                    // reset data counter
  memset(sensordata, 0, sizeof(sensordata));    // clear sensordata array;

  if (cmd[0] == 'c' || cmd[0] == 'r')time = 1400;
  else time = 300;                              //if a command has been sent to calibrate or take a reading we
  //wait 1400ms so that the circuit has time to take the reading.
  //if any other command has been sent we wait only 300ms.
  
  Wire.beginTransmission(channel); 	// call the circuit by its ID number.
  Wire.write(cmd);        		// transmit the command that was sent through the serial port.
  Wire.endTransmission();          	// end the I2C data transmission.

  delay(time);

  code = 254;				// init code value

  while (code == 254) {                 // in case the cammand takes longer to process, we keep looping here until we get a success or an error

    Wire.requestFrom(channel, 48, 1);   // call the circuit and request 48 bytes (this is more then we need).
    code = Wire.read();

    while (Wire.available()) {          // are there bytes to receive.
      in_char = Wire.read();            // receive a byte.

      if (in_char == 0) {               // if we see that we have been sent a null command.
        Wire.endTransmission();         // end the I2C data transmission.
        break;                          // exit the while loop.
      }
      else {
        sensordata[sensor_bytes_received] = in_char;  // load this byte into our array.
        sensor_bytes_received++;
      }
    }


    switch (code) {                  	// switch case based on what the response code is.
      case 1:                       	// decimal 1.
        Console.println("Success");  	// means the command was successful.
        break;                        	// exits the switch case.

      case 2:                        	// decimal 2.
        Console.println("< command failed");    	// means the command has failed.
        break;                         	// exits the switch case.

      case 254:                      	// decimal 254.
        Console.println("< command pending");   	// means the command has not yet been finished calculating.
        delay(200);                     // we wait for 200ms and give the circuit some time to complete the command
        break;                         	// exits the switch case.

      case 255:                      	// decimal 255.
        Console.println("No Data");   	// means there is no further data to send.
        break;                         	// exits the switch case.
    }

  }

  Console.println(sensordata);	        // print the data.
}
