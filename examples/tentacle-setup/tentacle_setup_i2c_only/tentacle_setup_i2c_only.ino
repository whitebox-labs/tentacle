// WhiteBox Labs -- Tentacle Shield -- Circuit Setup -- I2C only!
// https://www.whiteboxes.ch/tentacle
//
// NOTE: This sketch will work only with circuits in I2C mode, e.g. if you're using the Tentacle Mini
// or the Tentacle on an Arduino without SoftSerial (e.g. Zero)
//
// Tool to help you setup multiple sensor circuits from Atlas Scientific
// It will allow you to control up to 8 Atlas Scientific devices through the I2C bus
//
// THIS IS A TOOL TO SETUP YOUR CIRCUITS INTERACTIVELY. THIS CODE IS NOT INTENDED AS A BOILERPLATE FOR YOUR PROJECT.
//
// This code is intended to work on all Arduinos. If using the Arduino Yun, connect
// to it's serial port. If you want to work with the Yun wirelessly, check out the respective
// Yun version of this example.
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set host serial terminal to 9600 baud
// - To open a I2C address, send the number of the address
// - To issue a command, enter it directly to the console.
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

#include <Wire.h>                	        //enable I2C.


#ifdef ARDUINO_SAM_DUE  // Arduino Due  
#define WIRE Wire1  
#else 
#define WIRE Wire  
#endif 

char sensordata[30];                     //A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received = 0;        //We need to know how many characters bytes have been received
byte sensor_bytes_received = 0;          //We need to know how many characters bytes have been received
int channel;                             //INT pointer for channel switching - 0-7 serial, 8-127 I2C addresses
char *cmd;                               //Char pointer used in string parsing
int retries;                             // com-check functions store number of retries here
boolean answerReceived;                  // com-functions store here if a connection-attempt was successful
byte error;                              // error-byte to store result of Wire.transmissionEnd()

String stamp_type;                       // hold the name / type of the stamp
char stamp_version[4];                   // hold the version of the stamp

char computerdata[20];           	       //we make a 20 byte character array to hold incoming data from a pc/mac/other.

byte i2c_response_code = 0;              //used to hold the I2C response code.
byte in_char = 0;                  	     //used as a 1 byte buffer to store in bound bytes from an I2C stamp.



void setup() {

  Serial.begin(9600);                    // Set the hardware serial port to 38400
  while (!Serial) ;                      // Leonardo-type arduinos need this to be able to write to the serial port in setup()
  WIRE.begin();                 	       // enable I2C port.

  stamp_type.reserve(16);                // reserve string buffer to save some SRAM
  intro();				 // display startup message
}



void loop() {
  //Serial.println(".");
  if (computer_bytes_received != 0) {            //If input recieved from PC/MAC/other
    cmd = computerdata;                          //Set cmd with incoming serial data

    if (String(cmd) == "help") {		 //if help entered...
      help(); 					 //call help dialogue
      computer_bytes_received = 0;               //Reset the var computer_bytes_received to equal 0
      return;
    }
    else if (String(cmd) == "scan") {         // if scan requested
      scan(true);
      computer_bytes_received = 0;               // Reset the var computer_bytes_received to equal 0
      return;
    }
    else if (String(cmd) == "scani2c") {
      scan(false);
      computer_bytes_received = 0;               // Reset the var computer_bytes_received to equal 0
      return;
    }
    else {

      // TODO: without loop?
      for (int x = 0; x <= 127; x++) {		//loop through input searching for a channel change request (integer between 0 and 127)
        if (String(cmd) == String(x)) {
          Serial.print("changing channel to ");
          Serial.println( cmd);
          channel = atoi(cmd);			//set channel variable to number 0-127

          if (change_channel()) {		//set MUX switches or I2C address

            Serial.println(  "-------------------------------------");
            Serial.print(    "ACTIVE channel : ");
            Serial.println(  channel );
            Serial.print(    "Type: ");
            Serial.print(    stamp_type);
            Serial.print(    ", Version: ");
            Serial.print(    stamp_version);
            Serial.print(    ", COM: ");
            Serial.println(  "I2C");
          }
          else {
            Serial.println("CHANNEL NOT AVAILABLE! Empty slot? Not in I2C mode?");
            Serial.println("Try 'scan' or set baudrate manually (see 'help').");
          }
          computer_bytes_received = 0;                    //Reset the var computer_bytes_received to equal 0
          return;
        }
      }


      if (String(cmd).startsWith("i2c,")) {
        Serial.println("! when switching from i2c to serial or vice-versa,");
        Serial.println("! type the new ID to connect to the stamp.");
      } else if (String(cmd).startsWith("serial,")) {
        Serial.println("! switching to serial. This sketch does not support serial mode.");
        Serial.println("! use a serial compatible sketch (and arduino) to connect to the circuit.");
      }

    }

    Serial.print("> ");                              // echo to the serial console
    Serial.println(cmd);

    I2C_call();  // send i2c command and wait for answer
    if (sensor_bytes_received > 0) {
      Serial.print("< ");
      Serial.println(sensordata);       //print the data.
    }


    computer_bytes_received = 0;          //Reset the var computer_bytes_received to equal 0
  }

  if (Serial.available() > 0) {                           //If data has been transmitted from an Atlas Scientific device
    computer_bytes_received = Serial.readBytesUntil(13, computerdata, 20);   //We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received
    computerdata[computer_bytes_received] = 0;          //We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
  }

}



boolean change_channel() {                                 //function controls which I2C port is opened. returns true if channel could be changed.

  stamp_type = "";
  memset(stamp_version, 0, sizeof(stamp_version));         // clear stamp info

  for (int x = 1; x <= 127; x++) {
    if (channel == x) {
      if ( !check_i2c_connection() ) {                   // check if this I2C port can be opened
        return false;
      }
      return true;
    }
  }
  return false;
}



byte I2C_call() {  					//function to parse and call I2C commands.
  sensor_bytes_received = 0;                            // reset data counter
  memset(sensordata, 0, sizeof(sensordata));            // clear sensordata array;

  WIRE.beginTransmission(channel); 	                //call the circuit by its ID number.
  WIRE.write(cmd);        			        //transmit the command that was sent through the serial port.
  WIRE.endTransmission();          	                //end the I2C data transmission.

  i2c_response_code = 254;
  while (i2c_response_code == 254) {      // in case the cammand takes longer to process, we keep looping here until we get a success or an error

    if (String(cmd).startsWith("cal") || String(cmd).startsWith("Cal") ) {
      delay(1400);                        // cal-commands take 1300ms or more
    } else if (String(cmd) == "r" || String(cmd) == "R") {
      delay(1000);                        // reading command takes about a second
    }
    else {
      delay(300);                         // all other commands: wait 300ms
    }

    WIRE.requestFrom(channel, 32); 	  //call the circuit and request 48 bytes (this is more then we need).
    i2c_response_code = WIRE.read();      //the first byte is the response code, we read this separately.

    while (WIRE.available()) {            //are there bytes to receive.
      in_char = WIRE.read();              //receive a byte.

      if (in_char == 0) {                 //if we see that we have been sent a null command.
        while(WIRE.available()) { WIRE.read(); } // some arduinos (e.g. ZERO) put padding zeroes in the receiving buffer (up to the number of requested bytes)
        break;                            //exit the while loop.
      }
      else {
        sensordata[sensor_bytes_received] = in_char;        //load this byte into our array.
        sensor_bytes_received++;
      }
    }

    switch (i2c_response_code) {         //switch case based on what the response code is.
      case 1:                       	 //decimal 1.
        Serial.println( "< success");     //means the command was successful.
        break;                        	 //exits the switch case.

      case 2:                        	 //decimal 2.
        Serial.println( "< command failed");    	//means the command has failed.
        break;                         	 //exits the switch case.

      case 254:                      	 //decimal 254.
        Serial.println( "< command pending");   	//means the command has not yet been finished calculating.
        break;                         	 //exits the switch case.

      case 255:                      	 //decimal 255.
        Serial.println( "No Data");   	//means there is no further data to send.
        break;                         	 //exits the switch case.
    }
  }
}



boolean check_i2c_connection() {                      // check selected i2c channel/address. verify that it's working by requesting info about the stamp

  retries = 0;

  while (retries < 3) {
    retries++;
    WIRE.beginTransmission(channel);      // just do a short connection attempt without command to scan i2c for devices
    error = WIRE.endTransmission();

    if (error == 0)                       // if error is 0, there's a device
    {

      int r_retries = 0;
      while (r_retries < 3) {
        r_retries++;

        cmd = "i";                          // set cmd to request info (in I2C_call())
        I2C_call();

        if (parseInfo()) {
          return true;
        }
      }

      return false;
    }
    else
    {
      return false;                      // no device at this address
    }
  }
}



boolean parseInfo() {                  // parses the answer to a "i" command. returns true if answer was parseable, false if not.

  // example:
  // PH EZO  -> '?I,pH,1.1'
  // ORP EZO -> '?I,OR,1.0'   (-> wrong in documentation 'OR' instead of 'ORP')
  // DO EZO  -> '?I,D.O.,1.0' || '?I,DO,1.7' (-> exists in D.O. and DO form)
  // EC EZO  -> '?I,EC,1.0 '

  // Legazy PH  -> 'P,V5.0,5/13'
  // Legazy ORP -> 'O,V4.4,2/13'
  // Legazy DO  -> 'D,V5.0,1/13'
  // Legazy EC  -> 'E,V3.1,5/13'

  if (sensordata[0] == '?' && sensordata[1] == 'I') {          // seems to be an EZO stamp

    // PH EZO
    if (sensordata[3] == 'p' && sensordata[4] == 'H') {
      stamp_type = F("pH EZO");
      stamp_version[0] = sensordata[6];
      stamp_version[1] = sensordata[7];
      stamp_version[2] = sensordata[8];
      stamp_version[3] = 0;

      return true;

      // ORP EZO
    }
    else if (sensordata[3] == 'O' && sensordata[4] == 'R') {
      stamp_type = F("ORP EZO");
      stamp_version[0] = sensordata[6];
      stamp_version[1] = sensordata[7];
      stamp_version[2] = sensordata[8];
      stamp_version[3] = 0;
      return true;

      // DO EZO
    }
    else if (sensordata[3] == 'D' && sensordata[4] == 'O') {
      stamp_type = F("D.O. EZO");
      stamp_version[0] = sensordata[6];
      stamp_version[1] = sensordata[7];
      stamp_version[2] = sensordata[8];
      stamp_version[3] = 0;
      return true;

      // D.O. EZO
    }
    else if (sensordata[3] == 'D' && sensordata[4] == '.' && sensordata[5] == 'O' && sensordata[6] == '.') {
      stamp_type = F("D.O. EZO");
      stamp_version[0] = sensordata[8];
      stamp_version[1] = sensordata[9];
      stamp_version[2] = sensordata[10];
      stamp_version[3] = 0;
      return true;

      // EC EZO
    }
    else if (sensordata[3] == 'E' && sensordata[4] == 'C') {
      stamp_type = F("EC EZO");
      stamp_version[0] = sensordata[6];
      stamp_version[1] = sensordata[7];
      stamp_version[2] = sensordata[8];
      stamp_version[3] = 0;
      return true;

      // unknown EZO stamp
    }
    else {
      stamp_type = F("unknown EZO stamp");
      return true;
    }

  }

  // it's a legacy stamp (non-EZO)
  else
  {
    // Legacy pH
    if ( sensordata[0] == 'P') {
      stamp_type = F("pH (legacy)");
      stamp_version[0] = sensordata[3];
      stamp_version[1] = sensordata[4];
      stamp_version[2] = sensordata[5];
      stamp_version[3] = 0;
      return true;

      // legacy ORP
    }
    else if ( sensordata[0] == 'O') {
      stamp_type = F("ORP (legacy)");
      stamp_version[0] = sensordata[3];
      stamp_version[1] = sensordata[4];
      stamp_version[2] = sensordata[5];
      stamp_version[3] = 0;
      return true;

      // Legacy D.O.
    }
    else if ( sensordata[0] == 'D') {
      stamp_type = F("D.O. (legacy)");
      stamp_version[0] = sensordata[3];
      stamp_version[1] = sensordata[4];
      stamp_version[2] = sensordata[5];
      stamp_version[3] = 0;
      return true;

      // Lecagy EC
    }
    else if ( sensordata[0] == 'E') {
      stamp_type = F("EC (legacy)");
      stamp_version[0] = sensordata[3];
      stamp_version[1] = sensordata[4];
      stamp_version[2] = sensordata[5];
      stamp_version[3] = 0;
      return true;
    }
  }

  /*
    Serial.println("can not parse data: ");
    Serial.print("'");
    Serial.print(sensordata);
    Serial.println("'");
  */
  return false;        // can not parse this info-string
}



void scan(boolean scanserial) {                      // Scan for all devices. Set scanserial to false to scan I2C only (much faster!)

  Serial.println(F("Starting  I2C scan..."));

  int stamp_amount = 0;

  for (channel = 1; channel < 127; channel++ )
  {

    if (change_channel()) {
      stamp_amount++;

      serialPrintDivider();
      Serial.print(    F("-- I2C CHANNEL "));
      Serial.println(  channel);
      Serial.println(  F("--"));
      Serial.print(    F("-- Type: "));
      Serial.println(  stamp_type);
    }
  }

  Serial.println(  F("\r\r"));
  Serial.println(  F("SCAN COMPLETE"));
  Serial.print(    stamp_amount);
  Serial.println(  F(" stamps found"));
}



void intro() {                                 			       //print intro
  Serial.flush();
  serialPrintDivider();
  Serial.println( F("Whitebox Labs -- Tentacle Shield - Stamp Setup - I2C only"));
  Serial.println( F("For info type 'help'"));
  Serial.println( F("To read current config from attached stamps type 'scan'"));
  Serial.println( F("This program will not recognize circuits in serial/UART mode."));
  Serial.println( F("TYPE CHANNEL NUMBER (Serial: 0-7, I2C: 0-127):"));
}

void help() {                                 			     //print help dialogue
  serialPrintDivider();
  Serial.println( F("To open a I2C address (between 0 - 127), send the number of the address"));
  Serial.println( F("To issue a command, enter it directly to the console."));
  Serial.println( F("To update information about connected stamps, type 'scan'."));
  Serial.println( F("=========="));
}

void serialPrintDivider() {
  Serial.println(  F("------------------"));
}
