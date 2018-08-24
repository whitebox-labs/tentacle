# UART Multiplexer

The on-board UART multiplexer allows to communicate to 4 EZO Circuits in `UART` mode using only one TX/RX pair from the Arduino. The `UART` connection to the EZO Circuits are pins 11 and 10, RX and TX. You can use `SoftSerial` to address all circuits via these pins.

Only one EZO Circuit can be connected to the upstream `UART`(pins 11 and 10) at one time. By configuring the multiplexer with the respective pins, you decide which EZO circuit you currently talk to.

Pins 7 (S0) and 6 (S1) are used to configure the multiplexer. Depending on the combination of setting HIGH/LOW of these two pins, a different EZO channel is connected to the Ardunios `UART`.

Some code tells more than 1000 words:

```arduino
const int s0 = 7; //Arduino pin 7 to control pin S0
const int s1 = 6; //Arduino pin 6 to control pin S1
const int enable_1 = 5; //Arduino pin 5 to control pin E on shield 1
const int enable_2 = 4; //Arduino pin 4 to control pin E on shield 2

void change_serial_mux_channel() {           // configures the serial muxers depending on channel.

  switch (channel) {                         //Looking to see what channel to open

    case 0:                                  //If channel==0 then we open channel 0
      digitalWrite(enable_1, LOW);           //Setting enable_1 to low activates primary channels: 0,1,2,3
      digitalWrite(enable_2, HIGH);          //Setting enable_2 to high deactivates secondary channels: 4,5,6,7
      digitalWrite(s0, LOW);                 //S0 and S1 control what channel opens
      digitalWrite(s1, LOW);                 //S0 and S1 control what channel opens
      break;                                 //Exit switch case

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

    case 6:
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

    default:
      digitalWrite(enable_1, HIGH);		//disable soft serial
      digitalWrite(enable_2, HIGH);		//disable soft serial
  }
}
```

## Stacking Two Shields

Pins 5 and 4 are used to enable/disable the multiplexer. This is useful if you stack two Tentacle shields. With two shields, you can access up to 8 EZO circuits in UART mode. The multiplexer is EITHER enabled by pin 5, OR by pin 4:
* If the `Channel Group Jumper` is in the 0-3 position
 * the multiplexer is enabled via pin 5
* If the `Channel Group Jumper` is in 4-7 position
 * the multiplexer is enabled via pin 4

Only one multiplexer should enabled at one time (pin 5 OR pin 4, but not both)
