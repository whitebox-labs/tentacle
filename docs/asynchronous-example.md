# Continuous, Asynchronous Readings

For this project you'll need:
* Arduino, 5V tolerant
* 1 or 2 Tentacle Shield(s)
* 1-8 Atlas Scientific circuits
* Computer with Arduino IDE installed
* An LED on Arduino pin 13

Often the Arduino should do several things at the same time - reading a pH value and updating a display. Or reading the temperature and sending it to a web API. This code shows how to do several things asynchronously, without waiting and without using `delay()`. This means while we're waiting for new data from the EZO Circuit, the Arduino can do other things during this time. This code demonstrates this using a blinking LED, implemented without wait or delay.

You can find all code mentioned on this page, including a special Arduino Yún version of each sketch on [GitHub](https://github.com/whitebox-labs/tentacle-examples/tree/master/arduino/asynchronous).


## UART mode
1. Set all your circuits to UART mode. This is the circuits factory setting.
1. Attach an LED to pin 13 to see it blinking unaffected by the waiting for the sensor readings
1. Copy the code below to to your Arduino sketch
1. Adjust the variables in the code to resemble your setup (see the in-code comments for an explanation on how these work):
 * TOTAL_CIRCUITS
 * channel_ids
 * channel_names
1. Upload the code to your Arduino
1. Open the Arduino IDE serial monitor `@9600 baud`
1. See the stream of data coming in

[](https://raw.githubusercontent.com/whitebox-labs/tentacle-examples/master/arduino/asynchronous/uart_asynchronous/uart_asynchronous.ino ':include :type=code arduino')

[(View on GitHub)](https://github.com/whitebox-labs/tentacle-examples/blob/master/arduino/asynchronous/uart_asynchronous/uart_asynchronous.ino)

## I2C mode

1. Set all your circuits to I2C mode. Learn how to do this in our guide I2C or UART?
1. Set all circuits to a unique I2C address - this is important in case you have multiple of the same circuit type, e.g. 2x EZO pH circuit
1. Copy the code below to to your Arduino sketch
1. Adjust the variables in the code to resemble your setup  (see the in-code comments for an explanation on how these work):
 * TOTAL_CIRCUITS
 * channel_ids
 * channel_names
1. Upload the code to your Arduino
1. Open the Arduino IDE serial monitor `@9600 baud`
1. See the stream of data coming in

[](https://raw.githubusercontent.com/whitebox-labs/tentacle-examples/master/arduino/asynchronous/i2c_asynchronous/i2c_asynchronous.ino ':include :type=code arduino')

[(View on GitHub)](https://github.com/whitebox-labs/tentacle-examples/blob/master/arduino/asynchronous/i2c_asynchronous/i2c_asynchronous.ino)
