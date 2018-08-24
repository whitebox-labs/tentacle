# EZO Circuit Protocols

Most EZO Circuits come with two protocols: `UART` and `I2C`. Factory default is `UART`, at a baudrate of `9600`.


## Distinguish the protocols

### UART
* Green LED: `UART` mode
* Blink / fade to Cyan: taking a reading (every second in continuous mode)

![EZO UART](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/uart.gif)

### I2C
* Blue LED: `I2C` mode
* Cyan LED: taking a reading

![EZO I2C](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/i2c.gif)



## Switch protocol type manually
This procedure toggles the protocol between `UART` and `I2C`. If the EZO Circuit is in `UART` mode, this procedure will switch it to `I2C`. If it is in `I2C` mode, it will switch it to `UART`.

!> Before starting the procedure, remove the EZO Circuit from the Tentacle (or other carrier boards). Remove power and all connections.

?> This procedure is easiest using a breadboard and a set of jumper wires

1. Connect (shortcut) these two pins:
 * **PGND pin to the TX pin** if your circuit is `EZO pH`, `EZO DO`, `EZO ORP` or `EZO EC`
 * Only exception is `EZO RTD`: Short the **PRB pin to the TX pin** instead.
1. Power the EZO Circuit (GND, +5V)
 * Wait for LED to change from green to blue (`UART`->`I2C`) or from blue to green (`I2C`->`UART`).
 ![EZO Manual Toggle](https://github.com/whitebox-labs/tentacle/raw/master/docs/_media/manual_toggle.png)
 _(The Arduino is used as a power source only. You can connect any other power source (3.3V-5V))_
1. **Remove** the jumper wire from the PGND (or PRB respectively) pin to the TX pin
1. **Remove** power (GND, 5V)

?> The device is now using the new protocol (repeat above steps to toggle back to the other protocol)

### Factory defaults
If the EZO Circuit is manually switched to `I2C`, its ID is reset to the factory default:

* `EZO DO`: **97** _(0x60)_
* `EZO ORP`: **98** _(0x61)_
* `EZO pH`: **99** _(0x63)_
* `EZO EC`: **100** _(0x64)_
* `EZO RTD`: **102** _(0x66)_

If the EZO Circuit is manually switched to `UART`, its baudrate is reset to the factory default of `9600`.

## Switch protocol type with the press of a button

## Switch protocol type by code

To switch to `I2C`, use the command `i2c`:
```
i2c,111
```
111 is the new I2C address of the circuit. It can be 1-127 and must be unique on the same `I2C` bus.

To switch to `UART`, use the command `baud`:
```
baud,9600
```
9600 is the UART baudrate (can be: 300, 1200, 2400, 9600 _(default)_, 19200, 38400, 57600, 115200)

!> After changing the protocol, the microcontroller will lose connection EZO Circuit - due to the different nature of `UART` and `I2C` busses.

?> If you're using the Tentacle Shield for Arduino, you can change the `Protocol Jumpers` now to adjust the Tentacle to the new protocol.  

### Using the Tentacle for Arduino and the Setup Sketch
1. Load the [Tentacle Setup Sketch](https://raw.githubusercontent.com/whitebox-labs/tentacle-examples/master/arduino/tentacle-setup/tentacle_setup/tentacle_setup.ino ':target=_blank') to your Arduino
1. open the serial monitor `@9600`
1. Type the ID of the circuit and press enter. Try ‘scan’, if you don’t know the ID of your circuit.
 * to switch to `I2C` mode, type `i2c,111`
 * to switch to `UART` mode, type `baud,9600`
1. Change both `protocol jumpers` for this channel to the new protocol
1. you are able to connect to the new channel ID immediately, no need  to restart the Arduino



##  Protocol Lock (Plock)

To prevent accidental change of protocol in production, the protocol can be locked. While `Protocol Lock` is enabled, attempts to change the protocol by code or the manual method will fail. Only after disabling `Protocol Lock` the other methods will work again.
Using the `Plock` command:
```
Plock,1  // enable Plock
Plock,0  // disable Plock, default
Plock,?  // Plock on/off?
```
