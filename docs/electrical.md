# Electronic Specifications

## 5V only
* The Tentacle for Arduino is powered solely from the Arduino 5V line.
* The isolated communication lines (RX/TX, SCL/SDA) are pulled to 5V - ** use a 5V-tolerant Arduino** (most are)
* The EZO circuits on the isolated side are powered by 5V.

## Schematic
* [Download Schematic (PDF)](https://github.com/whitebox-labs/tentacle/raw/master/hardware/tentacle_schematic.pdf)
* [All Source Files (GitHub)](https://github.com/whitebox-labs/tentacle/)

## Power Consumption
* ~140mA @ 5V
 * that's ~35mA per channel, drawn at all times (even if no EZO Circuit present)
* Add the power consumption of your respective EZO Circuits @5V:
 * EZO pH: 57mA
 * EZO ORP: 58mA
 * EZO EC: 82mA
 * EZO DO: 53mA
 * EZO RTD: 59mA
