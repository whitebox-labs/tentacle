# Interactive, Bidirectional Communications

For this project you'll need:
* Arduino, 5V tolerant
* 1 or 2 Tentacle Shield(s)
* 1-8 Atlas Scientific circuits
* Computer with Arduino IDE installed

This example demonstrates how to establish interactive, bidirectional communications to the EZO circuits through the Arduino USB/Serial connection.

You can find all code mentioned on this page, including a special Arduino Yún version of each sketch on [GitHub](https://github.com/whitebox-labs/tentacle-examples/tree/master/arduino/interactive).


## UART mode
1. Set all your circuits to UART mode. This is the circuits factory setting.
1. Copy the code below to to your Arduino sketch
1. Upload the code to your Arduino
1. Open the Arduino IDE serial monitor `@9600 baud`
1. Type the channel number, a colon and the command into the serial monitor like so:
`0:r` or `0:i`

[](https://raw.githubusercontent.com/whitebox-labs/tentacle-examples/master/arduino/interactive/uart_interactive/uart_interactive.ino ':include :type=code arduino')

[(View on GitHub)](https://github.com/whitebox-labs/tentacle-examples/blob/master/arduino/interactive/uart_interactive/uart_interactive.ino)

## I2C mode

1. Set all your circuits to I2C mode. Learn how to do this in our guide I2C or UART?
1. Set all circuits to a unique I2C address - this is important in case you have multiple of the same circuit type, e.g. 2x EZO pH circuit
1. Copy the code below to to your Arduino sketch
1. Upload the code to your Arduino
1. Open the Arduino IDE serial monitor `@9600 baud`
1. Type the channel number, a colon and the command into the serial monitor like so:
`99:r` or `99:i`

[](https://raw.githubusercontent.com/whitebox-labs/tentacle-examples/master/arduino/interactive/i2c_interactive/i2c_interactive.ino ':include :type=code arduino')

[(View on GitHub)](https://github.com/whitebox-labs/tentacle-examples/blob/master/arduino/interactive/i2c_interactive/i2c_interactive.ino)
