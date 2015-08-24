
//WhiteBox Labs -- Tentacle Shield -- Version 1.0

//Soft serial code based on Atlas Scientific reference designs found here:
//https://www.atlas-scientific.com/_files/code/serial_port_connector_arduino_uno_2.pdf
//updated to support native Arduino 1.0 Soft Serial on multiple platforms.
//updated to support I2C functionality for general purpose usage with dynamic response time.
//This sample code was written on an Arduino MEGA, with cross-compatibility for UNO, Leonardo in mind.
//It will allow you to control up to 8 Atlas Scientific devices through 1 soft serial RX/TX line or the I2C bus
//-------------------------------------------------------------------------------------------------------------
//Set host serial terminal to 9600 baud
//Set Atlas Scientific devices to 9600 baud (or change serial_ch* values below)
//-------------------------------------------------------------------------------------------------------------
//To open a serial channel (numbered 0 - 7), send the number of the channel
//To open a I2C address (between 8 - 127), send the number of the address
//To issue a command, enter it directly to the console.


#include <SoftwareSerial.h>         //Include the software serial library  
#include <Wire.h>                	//enable I2C.

SoftwareSerial sftSerial(11, 10);   //RX, TX  - Name the software serial library sftSerial (this cannot be omitted)  
                                    //assigned to pins 10 and 11 for maximum compatibility

									
unsigned long serial_host = 9600;	//set baud rate for host serial monitor(pc/mac/other)

unsigned long serial_ch0 = 38400;	//set baudrates for serial channels 0-7.
unsigned long serial_ch1 = 38400;	//9600 (recommended) is the default rate for Atlas Scientific EZO stamps shipped after November 6th 2014
unsigned long serial_ch2 = 38400;	//38400 is the default rate EZO and legacy devices shipped before this date.
unsigned long serial_ch3 = 38400;	//other values allowed: 300,1200,2400,9600,19200,38400,57600,155200 
unsigned long serial_ch4 = 38400;	//See the datasheet for your Atlas Scientific device for instructions on changing the default baud rate.
unsigned long serial_ch5 = 38400;
unsigned long serial_ch6 = 38400;
unsigned long serial_ch7 = 38400;

int s0 = 7;                         //Arduino pin 7 to control pin S0
int s1 = 6;                         //Arduino pin 6 to control pin S1
int enable_1= 5;	            	//Arduino pin 5 to control pin E on board 1
int enable_2= 4;                    //Arduino pin 4 to control pin E on board 2

char sensordata[30];                //A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received=0;     //We need to know how many characters bytes have been received         
byte sensor_bytes_received=0;       //We need to know how many characters bytes have been received
int channel;                       	//INT pointer for channel switching - 0-7 serial, 8-127 I2C addresses
char *cmd;                          //Char pointer used in string parsing

char computerdata[20];           	//we make a 20 byte character array to hold incoming data from a pc/mac/other.   
byte received_from_computer=0;   	//we need to know how many characters have been received.    
byte code=0;                     	//used to hold the I2C response code. 
char i2c_sensordata[48];           	//we make a 48 byte character array to hold incoming data from the EC circuit. 
byte in_char=0;                  	//used as a 1 byte buffer to store in bound bytes from the EC Circuit.   
byte i=0;                        	//counter used for i2c_sensordata array. 
int time;                   		//used to change the dynamic polling delay needed for I2C read operations.

boolean I2C_mode=false;				//bool switch for serial/I2C
int address=100;					//INT for I2C address


void setup(){                                 			//startup function
	pinMode(s1, OUTPUT);				//Set the digital pin as output.
	pinMode(s0, OUTPUT);				//Set the digital pin as output.
	pinMode(enable_1, OUTPUT);			//Set the digital pin as output.
	pinMode(enable_2, OUTPUT);			//Set the digital pin as output. 
	Serial.begin(serial_host);			//Set the hardware serial port.
	sftSerial.begin(serial_ch0);		//Set the soft serial port to rate of default channel (0).
	Wire.begin();						//enable I2C port.
	intro();							//display startup message
  }

void serialEvent(){               						//This interrupt will trigger when the data coming from the serial monitor(pc/mac/other) is received   
           computer_bytes_received=Serial.readBytesUntil(13,computerdata,20); 		//We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received    
           computerdata[computer_bytes_received]=0; 								//We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
           }    
         
void loop(){                                 			//main loop
     
    if(computer_bytes_received!=0){                 	//If input received from PC/MAC/other 
        cmd=computerdata;                             	//Set cmd with incoming serial data
		
		if(String(cmd)=="help"){			  			//if help entered...
			help(); 								  	//call help dialogue
			computer_bytes_received=0;                	//Reset the var computer_bytes_received to equal 0
			return;
		}
		
		for (int x = 0; x <= 127; x++) {				//loop through input searching for a channel change request (integer between 0 and 127)
			if(String(cmd)==String(x)){
				channel=atoi(cmd);						//set channel variable to number 0-127
				change_channel();						//set MUX switches or I2C address
				computer_bytes_received=0;              //Reset the var computer_bytes_received to equal 0
                Serial.println("Channel changed: " + String(cmd));
				return;
			}
		}
		
		if(I2C_mode==false){							//if serial channel selected
            //Serial.print("> ");
			//Serial.println(cmd);		              	//echo command to user
			sftSerial.print(cmd);                       //Send the command from the computer to the Atlas Scientific device using the softserial port 
			sftSerial.print("\r");                      //After we send the command we send a carriage return <CR> 
		}
		else{											//if I2C address selected
			I2C_call();									//send to I2C
		}
		computer_bytes_received=0;                  	//Reset the var computer_bytes_received to equal 0
    }
  
	if(sftSerial.available() > 0){                   						//If data has been transmitted from an Atlas Scientific device
		sensor_bytes_received=sftSerial.readBytesUntil(13,sensordata,30); 	//we read the data sent from the Atlas Scientific device until we see a <CR>. We also count how many character have been received 
		sensordata[sensor_bytes_received]=0;           						//we add a 0 to the spot in the array just after the last character we received. This will stop us from transmitting incorrect data that may have been left in the buffer
		//Serial.print("< ");
        Serial.println(sensordata);                    						//let’s transmit the data received from the Atlas Scientific device to the serial monitor   
	}  
}

void intro(){                                 			//print intro
	Serial.flush();
	Serial.println(" ");
	Serial.println("WhiteBox Labs -- Tentacle Shield -- Version 1.0");
	Serial.println("For info type 'help'           				   ");
	Serial.println("READY_                                         ");
}  

void help(){                                 			//print help dialogue
	Serial.println("To open a serial channel (numbered 0 - 7), send the number of the channel");
	Serial.println("To open a I2C address (between 8 - 127), send the number of the address");
	Serial.println("To issue a command, enter it directly to the console.");
	Serial.println("---------------------------------");
	Serial.println("Serial baud rates:");
	Serial.println("Channel 0 : " + String(serial_ch0));
	Serial.println("Channel 1 : " + String(serial_ch1));
	Serial.println("Channel 2 : " + String(serial_ch2));
	Serial.println("Channel 3 : " + String(serial_ch3));
	Serial.println("Channel 4 : " + String(serial_ch4));
	Serial.println("Channel 5 : " + String(serial_ch5));
	Serial.println("Channel 6 : " + String(serial_ch6));
	Serial.println("Channel 7 : " + String(serial_ch7));
}  
  
void change_channel(){                                 	//function controls which UART/I2C port is opened. 
    I2C_mode=false;									 	//0 for serial, 1 for I2C
    switch (channel) {                               	//Looking to see what channel to open 
	 
     case 0:                                       	 	//If channel==0 then we open channel 0     
         digitalWrite(enable_1,LOW);                 	//Setting enable_1 to low activates primary channels: 0,1,2,3  
         digitalWrite(enable_2,HIGH);                	//Setting enable_2 to high deactivates secondary channels: 4,5,6,7
         digitalWrite(s0, LOW);                      	//S0 and S1 control what channel opens 
         digitalWrite(s1, LOW);                      	//S0 and S1 control what channel opens
		 sftSerial.begin(serial_ch0);					//reset soft serial to baudrate defined for this channel
       break;                                        	//Exit switch case
        
       case 1:
         digitalWrite(enable_1,LOW);
         digitalWrite(enable_2,HIGH);
         digitalWrite(s0, HIGH);
         digitalWrite(s1, LOW);
		 sftSerial.begin(serial_ch1);
       break;

       case 2:
         digitalWrite(enable_1,LOW);
         digitalWrite(enable_2,HIGH);
         digitalWrite(s0, LOW);
         digitalWrite(s1, HIGH);
		 sftSerial.begin(serial_ch2);
       break;

       case 3:
         digitalWrite(enable_1,LOW);
         digitalWrite(enable_2,HIGH);
         digitalWrite(s0, HIGH);
         digitalWrite(s1, HIGH);
		 sftSerial.begin(serial_ch3);
       break;
       
        case 4:
         digitalWrite(enable_1,HIGH);
         digitalWrite(enable_2,LOW);
         digitalWrite(s0, LOW);                      
         digitalWrite(s1, LOW);
		 sftSerial.begin(serial_ch4);
       break;
       
        case 5:
         digitalWrite(enable_1,HIGH);
         digitalWrite(enable_2,LOW);
         digitalWrite(s0, HIGH);
         digitalWrite(s1, LOW);
		 sftSerial.begin(serial_ch5);
       break;
       
        case 6:
         digitalWrite(enable_1,HIGH);
         digitalWrite(enable_2,LOW);
         digitalWrite(s0, LOW);
         digitalWrite(s1, HIGH);
		 sftSerial.begin(serial_ch6);
       break;
       
        case 7:
         digitalWrite(enable_1,HIGH);
         digitalWrite(enable_2,LOW);
         digitalWrite(s0, HIGH);
         digitalWrite(s1, HIGH);
		 sftSerial.begin(serial_ch7);
       break;
       
		default:											//I2C mode
         digitalWrite(enable_1,HIGH);						//disable soft serial
         digitalWrite(enable_2,HIGH);						//disable soft serial
		 
		for (int x = 8; x <= 127; x++) {
			if(channel==x){
				address=x;
				//Serial.println("I2C");					//indicate port change
				//Serial.println(x);
				I2C_mode=true;								//0 for serial, 1 for I2C
				return;
			}
		}
    }
 }
 
void I2C_call(){  										//function to parse and call I2C commands

	//Serial.print("> ");                     			//echo command to user
    //Serial.println(cmd);
        
	Wire.beginTransmission(address); 	//call the circuit by its ID number.  
	Wire.write(cmd);        			//transmit the command that was sent through the serial port.
	Wire.endTransmission();          	//end the I2C data transmission. 
	
	code=0;								//init code value
	time=100;							//set time between checks for I2C reply
	for (int x = 0; x <= 20; x++) {     //check every 200ms for 2 seconds for a I2C response
		delay(time);					//delay 100ms * 20 iterations
		Wire.requestFrom(address,48,1); //call the circuit and request 48 bytes (this is more then we need).
		code=Wire.read();               //the first byte is the response code, we read this separately.  
		if(code==1)break;				//break loop if condition met
	}

	switch (code){                  	//switch case based on what the response code is.  
	  case 1:                       	//decimal 1.  
		//Serial.println("Success");  	//means the command was successful.
	  break;                        	//exits the switch case.

	 case 2:                        	//decimal 2. 
	   Serial.println("< command failed");    	//means the command has failed.
	 break;                         	//exits the switch case.

	 case 254:                      	//decimal 254.
	   Serial.println("< command pending");   	//means the command has not yet been finished calculating.
	 break;                         	//exits the switch case.
	 
	 case 255:                      	//decimal 255.
	   Serial.println("No Data");   	//means there is no further data to send.
	 break;                         	//exits the switch case.
	}

	while(Wire.available()){          	//are there bytes to receive.  
		in_char = Wire.read();          //receive a byte.
		i2c_sensordata[i]= in_char;		//load this byte into our array.
		i+=1;                           //incur the counter for the array element. 
		if(in_char==0){                 //if we see that we have been sent a null command. 
			i=0;                        //reset the counter i to 0.
			Wire.endTransmission();     //end the I2C data transmission.
			break;                      //exit the while loop.
		}
	}
		//Serial.print("< ");
		Serial.println(i2c_sensordata);	//print the data.  
	}
