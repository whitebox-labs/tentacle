# Pinout

The Tentacle Shield connects to 5V, GND, SCL, SDA. In addition some I/O pins are used for the UART multiplexer. If you remove the `channel group jumper`, you'll free up these pins - but your circuits must then be in I2C mode. See [Jumper Settings](jumpers.md) to learn what all the jumpers do.

?> The pins used by the Tentacle are marked black on the silkscreen of the Tentacle PCB, right outside the Arduino header.

## UART 0-3 enabled
The `channel group jumper` is placed in the 0-3 position.

![Pinout UART 0-3](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/pinout_uart1.png)

* SCL: I2C
* SDA: I2C
* 10: TX
* 11: RX
* 7: MUX S0
* 6: MUX S1
* 5: MUX ENABLE 1
* 4: **free**

## UART 4-7 enabled
The `channel group jumper` is placed in the 4-7 position.

![Pinout UART 0-3](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/pinout_uart2.png)

* SCL: I2C
* SDA: I2C
* 10: TX
* 11: RX
* 7: MUX S0
* 6: MUX S1
* 5: **free**
* 4: MUX ENABLE 2

## I2C only
The `channel group jumper` is removed. The multiplexer and UART is disabled.

![Pinout UART 0-3](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/pinout_i2c.png)

* SCL: I2C
* SDA: I2C
* 10: **free**
* 11: **free**
* 7: **free**
* 6: **free**
* 5: **free**
* 4: **free**
