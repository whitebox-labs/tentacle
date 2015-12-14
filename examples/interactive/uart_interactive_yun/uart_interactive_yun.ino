// WhiteBox Labs -- Tentacle Shield & Arduino Yun --  UART example
// www.whiteboxes.ch
//
// This code is based on https://www.atlas-scientific.com/_files/code/4-port-board.pdf
//
// How to use 4 (or 8 if using 2 Tentacle shields) Atlas Scientivic devices in serial mode.
// This sketch assumes all of your devices are either older serial devices or EZO circuits
// in serial mode. You can use the tentacle_setup.ino to autodetect and setup your devices.
//
// This sample code was written on an Arduino YUN, and depends on it's Bridge library to
// communicate wirelessly.
// For Arduino Mega, Uno etc, see the respective examples.
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - To talk to the Yun console, select your Yun's name and IP address in the Port menu.
//    - The Yun will only show up in the Ports menu, if your computer is on the same Network as the Yun.
//
// - Serial channel numbers are 0-3
//   ( Channels 4-7 are also available, if you're using two stacked Tentacle shields)
//
// - To open a channel, send the number of the channel, a colon and the command ending with a carriage return.
//
// 0:r<CR>
// 1:i<CR>
// 2:c<CR>
// 3:r<CR>
//
//---------------------------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------------------------

#include <SoftwareSerial.h>         // software serial library
#include <Console.h>                // Yun Console

SoftwareSerial sSerial(11, 10);     // RX, TX  - Name the software serial library sftSerial (this cannot be omitted)
                                    // assigned to pins 10 and 11 for maximum compatibility
int s0 = 7;                         // Tentacle uses pin 7 for multiplexer control S0
int s1 = 6;                         // Tentacle uses pin 6 for multiplexer control S1
int enable_1 = 5;	            // Tentacle uses pin 5 to control pin E on shield 1
int enable_2 = 4;	            // Tentacle uses pin 4 to control pin E on shield 2

char computerdata[20];              // A 20 byte character array to hold incoming data from a pc/mac/other
byte computer_bytes_received = 0;   // We need to know how many characters bytes have been received
int computer_in_byte;               // a variable to read incoming console data into
boolean computer_msg_complete = false;

char sensordata[30];                // a 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;     // we need to know how many characters bytes have been received

char *channel;                      // char pointer used in string parsing
char *cmd;                          // char pointer used in string parsing



void setup() {
  pinMode(s0, OUTPUT);             // set the digital output pins for the serial multiplexer
  pinMode(s1, OUTPUT);
  pinMode(enable_1, OUTPUT);
  pinMode(enable_2, OUTPUT);

  Bridge.begin();
  Console.begin();                 // initialize serial communication over network:
  while (!Console) ;               // wait for Console port to connect.
  sSerial.begin(9600);             // set the soft serial port to 9600
  intro();                         // display startup message
}


void loop() {
  
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
    channel = strtok(computerdata, ":");        // let's parse the string at each colon
    cmd = strtok(NULL, ":");
    open_channel();                             // call the function "open_channel" to open the correct multiplexed serial channel
    if (cmd != 0) {                             // if no command has been sent, send nothing
      sSerial.print(cmd);                       // send the command from the computer to the Atlas Scientific device using the softserial port
      sSerial.print("\r");                      // <CR> carriage return to terminate message
    }
    computer_msg_complete = false;              // reset the var computer_msg_complete to be ready for the next command
  }

  if (sSerial.available() > 0) {                 // if data has been transmitted from an Atlas Scientific device
    sensor_bytes_received = sSerial.readBytesUntil(13, sensordata, 30); //we read the data sent from the Atlas Scientific device until we see a <CR>. We also count how many character have been received
    sensordata[sensor_bytes_received] = 0;       // we add a 0 to the spot in the array just after the last character we received. This will stop us from transmitting incorrect data that may have been left in the buffer
    Console.println(sensordata);                 // let’s transmit the data received from the Atlas Scientific device to the serial monitor
  }
}


// Open a channel via the Tentacle serial multiplexer
void open_channel() {

  switch (*channel) {

    case '0':                                // if channel==0 then we open channel 0
      digitalWrite(enable_1, LOW);           // setting enable_1 to low activates primary channels: 0,1,2,3
      digitalWrite(enable_2, HIGH);          // setting enable_2 to high deactivates secondary channels: 4,5,6,7
      digitalWrite(s0, LOW);                 // S0 and S1 control what channel opens
      digitalWrite(s1, LOW);                 // S0 and S1 control what channel opens
      break;

    case '1':
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      break;

    case '2':
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      break;

    case '3':
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      break;

    case '4':
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, LOW);
      break;

    case '5':
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

    case '7':
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      break;
  }
}


// Print intro
void intro() {
  Console.flush();
  Console.println(" ");
  Console.println("READY_");
}
