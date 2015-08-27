
// WhiteBox Labs -- Tentacle Shield -- Mixed I2C and UART example

// Soft serial code based on Atlas Scientific reference designs found here:
// https://www.atlas-scientific.com/_files/code/serial_port_connector_arduino_uno_2.pdf
// updated to support native Arduino 1.0 Soft Serial on multiple platforms.
// updated to support I2C functionality for general purpose usage with dynamic response time.
// This sample code was written on an Arduino MEGA, with cross-compatibility for UNO, Leonardo in mind.
// It will allow you to control up to 8 Atlas Scientific devices through 1 soft serial RX/TX line or the I2C bus
//-------------------------------------------------------------------------------------------------------------
// Set host serial terminal to 9600 baud
// Set Atlas Scientific devices to 9600 baud (or change baudrate_ch* values below)
//------------------------------------------------------------------------------------------------------------
// To send a command, send the number of the channel,
// a colon and the command ending with a carriage return.
// serial channels are  numbered 0 - 7
// i2c addresses are numbered 8 - 127
// To issue a command, enter it directly to the console.
//
// 1:r<CR>
// 2:i<CR>
// 3:c<CR>
// 99:r<CR>
// 110:cal,mid,7.00<CR>


#include <SoftwareSerial.h>      // Include the software serial library  
#include <Wire.h>                // enable I2C.

SoftwareSerial sSerial(11, 10);  // RX, TX  - Name the software serial library sSerial (this cannot be omitted)
//assigned to pins 10 and 11 for maximum compatibility

unsigned long serial_host  = 9600;	// set baud rate for host serial monitor(pc/mac/other)

unsigned long baudrate_ch0 = 9600;	// set baudrates for serial channels 0-7.
unsigned long baudrate_ch1 = 9600;	// 9600 (recommended) is the default rate for Atlas Scientific EZO stamps shipped after November 6th 2014
unsigned long baudrate_ch2 = 9600;	// 38400 is the default rate EZO and legacy devices shipped before this date.
unsigned long baudrate_ch3 = 9600;	// other values allowed: 300,1200,2400,9600,19200,38400,57600,115200
unsigned long baudrate_ch4 = 9600;	// See the datasheet for your Atlas Scientific device for instructions on changing the default baud rate.
unsigned long baudrate_ch5 = 9600;
unsigned long baudrate_ch6 = 9600;
unsigned long baudrate_ch7 = 9600;

int s0 = 7;                         // Arduino pin 7 to control pin S0
int s1 = 6;                         // Arduino pin 6 to control pin S1
int enable_1 = 5;	            // Arduino pin 5 to control pin E on board 1
int enable_2 = 4;                   // Arduino pin 4 to control pin E on board 2

char sensordata[30];                // A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received = 0;   // We need to know how many characters bytes have been received
byte sensor_bytes_received = 0;     // We need to know how many characters bytes have been received
int channel;                        // INT pointer for channel switching - 0-7 serial, 8-127 I2C addresses
char *cmd;                          //Char pointer used in string parsing

char computerdata[48];              // we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte received_from_computer = 0;    // we need to know how many characters have been received.
byte code = 0;                      // used to hold the I2C response code.
byte in_char = 0;                   // used as a 1 byte buffer to store in bound bytes from the EC Circuit.
int time;                   	    // used to change the dynamic polling delay needed for I2C read operations.

boolean I2C_mode = false;	    // bool switch for serial/I2C


void setup() {                      // startup function
  pinMode(s1, OUTPUT);		    // Set the digital pin as output.
  pinMode(s0, OUTPUT);	            // Set the digital pin as output.
  pinMode(enable_1, OUTPUT);	    // Set the digital pin as output.
  pinMode(enable_2, OUTPUT);	    // Set the digital pin as output.
  Serial.begin(serial_host);	    // Set the hardware serial port.
  sSerial.begin(baudrate_ch0);	    // Set the soft serial port to rate of default channel (0).
  Wire.begin();			    // enable I2C port.
  intro();			    // display startup message
}


void serialEvent() {               						// This interrupt will trigger when the data coming from the serial monitor(pc/mac/other) is received
  computer_bytes_received = Serial.readBytesUntil(13, computerdata, 20); 	// We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received
  computerdata[computer_bytes_received] = 0; 				        // We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
}


void loop() {                                 	// main loop

  if (computer_bytes_received != 0) {           // If computer_bytes_received does not equal zero
    channel = atoi(strtok(computerdata, ":"));  // Let's parse the string at each colon
    cmd = strtok(NULL, ":");                    // Let's parse the string at each colon
    open_channel();                             // Call the function "open_channel" to open the correct data path

    if (I2C_mode == false) {	  // if serial channel selected
      sSerial.print(cmd);         // Send the command from the computer to the Atlas Scientific device using the softserial port
      sSerial.print("\r");        // After we send the command we send a carriage return <CR>
    } else {			  // if I2C address selected
      I2C_call();		  // send to I2C
    }

    computer_bytes_received = 0;  // Reset the var computer_bytes_received to equal 0
  }

  if (sSerial.available() > 0) {                   				// If data has been transmitted from an Atlas Scientific device
    sensor_bytes_received = sSerial.readBytesUntil(13, sensordata, 30); 	// we read the data sent from the Atlas Scientific device until we see a <CR>. We also count how many character have been received
    sensordata[sensor_bytes_received] = 0;           				// we add a 0 to the spot in the array just after the last character we received. This will stop us from transmitting incorrect data that may have been left in the buffer
    Serial.println(sensordata);                    				// let’s transmit the data received from the Atlas Scientific device to the serial monitor
  }
}

void intro() {                                // print intro
  Serial.flush();
  Serial.println(" ");
  Serial.println("READY_");
}


void open_channel() {                         // function controls which UART/I2C port is opened.
  I2C_mode = false;			      // false for serial, true for I2C
  switch (channel) {                          // Looking to see what channel to open

    case 0:                                   // If channel==0 then we open channel 0
      digitalWrite(enable_1, LOW);            // Setting enable_1 to low activates primary channels: 0,1,2,3
      digitalWrite(enable_2, HIGH);           // Setting enable_2 to high deactivates secondary channels: 4,5,6,7
      digitalWrite(s0, LOW);                  // S0 and S1 control what channel opens
      digitalWrite(s1, LOW);                  // S0 and S1 control what channel opens
      sSerial.begin(baudrate_ch0);	      // reset soft serial to baudrate defined for this channel
      break;                                  // Exit switch case

    case 1:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      sSerial.begin(baudrate_ch1);
      break;

    case 2:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      sSerial.begin(baudrate_ch2);
      break;

    case 3:
      digitalWrite(enable_1, LOW);
      digitalWrite(enable_2, HIGH);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      sSerial.begin(baudrate_ch3);
      break;

    case 4:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, LOW);
      sSerial.begin(baudrate_ch4);
      break;

    case 5:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      sSerial.begin(baudrate_ch5);
      break;

    case 6:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      sSerial.begin(baudrate_ch6);
      break;

    case 7:
      digitalWrite(enable_1, HIGH);
      digitalWrite(enable_2, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      sSerial.begin(baudrate_ch7);
      break;

    default:					// I2C mode
      digitalWrite(enable_1, HIGH);		// disable soft serial
      digitalWrite(enable_2, HIGH);		// disable soft serial

      if (channel <= 127) {
        I2C_mode = true;			// 0 for serial, 1 for I2C
        return;
      }

  }
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

  while (code == 254) {                 // in case the cammand takes longer to process, we

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
        //Serial.println("Success");  	// means the command was successful.
        break;                        	// exits the switch case.

      case 2:                        	// decimal 2.
        Serial.println("< command failed");    	// means the command has failed.
        break;                         	// exits the switch case.

      case 254:                      	// decimal 254.
        Serial.println("< command pending");   	// means the command has not yet been finished calculating.
        delay(200);                     // we wait for 200ms and give the circuit some time to complete the command
        break;                         	// exits the switch case.

      case 255:                      	// decimal 255.
        Serial.println("No Data");   	// means there is no further data to send.
        break;                         	// exits the switch case.
    }

  }

  Serial.println(sensordata);	// print the data.
}
