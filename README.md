# My Raspberry Pi Pico weather station

## Salient Points

- There are many of these, this one is mine.
- It is my first pico project.
- It will use LORA to speak with an intermittently available gateway
  and store the observed data until it can be sent.
- Windspeed, direction, temperature, humidity, and pressure. I probably
  won't do rain, they just get plugged up in my environment.
- Build host is a Raspberry Pi running Raspbian. Plain old tools, nothing fancy.
- You can plug a console into the USB to configure and interrogate it.

## Status

This is in current development. 

The first development board was destroyed when a cat rubbed herself on
it while it was in my test vise. I suspect static damage to the
RP2040. It got flakey, break the USB subsytem of an RP4 if plugged in,
but works fine on a mac, eventually I could flash it, verify the flash
with picotool, but it would only start the program if I left it
unplugged for an extended period of time. RIP prototype #1.

Prototype #2 dispenses with the DHT11, its mostly useless as a backup
since the RTC's thermometer is as accurate and the humidity sensor is
pretty much garbage.

## Peripherals

- BME280 pressure, humidity, temperature. The temperature accuracy isn't great.
  and is subject to self-heating.
- Anemometer and direction froma LaCrosse TX23U wind sensor. This will need
  a PIO state machine to record the bit stream timing for
  decoding. See
  https://www.john.geek.nz/2012/08/la-crosse-tx23u-anemometer-communication-protocol/
  for a protocol description and pinout.
- Maybe three or so one-wire temperature sensors to put on sunny side, shady side,
  and one in the ground?
- Hot wire anemometer? I could pulse the wire then measure the impedance decay?
  Calibrate it against the cup anemometer until the cup anemometer breaks.
- Trying the G-NiceRF 100mW LORA 915MHz module. https://www.nicerf.com/products/detail/100mw-lora-wireless-transceiver-module-lora1276-915.html But I may not be able to solder it. It is smaller than I thought and the eyes aren't what they were.
- DS3231 RTC on I2C, with AT24C32 4kB EEPROM. From https://www.amazon.com/gp/product/B07V68443F I may use the EEPROM for the device configuration.

## Wiring

### I2C 0

- SDA on GP20
- SCL in GP21
- DS3231 RTC address is 0x68 (probably)
- AT24C32 EEPROM address is 0x57 (probably)
- BM280 addess is 0x77 (this is the non-default alternate, but it's what answers on my board)

### TX23U

- This goes to the RJ45 jack. The protocol reverse engineering refers
  to a "brown" wire, which I assume is the RJ11 "black".
  
  | RJ11   | RJ45 T568A   | TX23U |
  |--------|--------------|-------|
  | Yellow | Orange/white | GND   |
  | Green  | Blue         | N.C.  |
  | Red    | Blue/white   | Vcc   |
  | Black  | Orange       | TxD   |

  **Note:** I *really* through the RJ11 colors should go the other way
  from my looking at wiring pictures on the internet, but the
  continuity tester doesn't lie, and this is working for me.
  
- Pin is GP22

### G-NiceRF 100mW LORA 915MHz module

- This goes on SPI1 for convenient wiring.
- There is a chipselect in the pinouts for SPI on the pico, but the
  SDK doesn't mention it. The tutorials I see online use that pin, but
  set it manually before doing blocking IO. I suspect the hardware can
  do something with it that doesn't show in the SDK. I'll use it for
  chip select and hope for a better future.
- I am connecting the LORA RESET line to a pin so I can reset the
  device from the processor if needed. The SX1276 docs say to leave it
  floating. Pull it low for 100uS to do a reset then wait 5ms, but
  elsewhere it says to wait 10ms before using SPI, so I'll go with 10ms.
- I'm using the D0 pin from the module to hopefully get the RX,TX, and
  maybe CADDone interrupts.
- I reworked an antenna pigtail to solder to the board, but I think I
  might rather just take the helical coil out of the duck antenna and
  mount it straight to the module. This will need a little testing to
  make sure it still performs well.
  
  | Module | Name   | Pico GPIO | Pico Pin |
  |--------|--------|-----------|----------|
  | 1      | GND    |           | rail     |
  | 2      | N.C.   |           |          |
  | 3      | N.C.   |           |          |
  | 4      | SCK    | GP10      | 14       |
  | 5      | MISO   | GP12      | 16       |
  | 6      | MOSI   | GP11      | 15       |
  | 7      | NSS    | GP13      | 17       |
  | 8      | DIO2   |           |          |
  | 9      | DIO1   |           |          |
  | 10     | DIO0   | GP14      | 19       |
  | 11     | Vcc    |           | rail     |
  | 12     | NRESET | GP9       | 12       |
  
## TODO

- **DONE** Remove DHT11 support, reuse GP22 for TX23U.
- **DONE** Add a 'time 2021-01-01 12:34:56' command to set time in rtc
- **DONE** Make set RTC also set DS3231 (blocking is ok)
- RTC returns bad time on first read in `time` command.
- Make the sampling cycle
  - **DONE** Function to get DS3231 into datetime_t (blocking is ok)
  - **DONE** Load RTC from DS3231 on boot
  - **DONE** Use system time to run sample look
  - Got some sort of sleep into main loop to lower power
  - Add DS3231 temperature to sampling cycle
  - **DONE** Pull in BME280 decoding from sdk examples
  - **DONE** Add BME280 decoding to sampling cycle (blocking for now)
- Support TX23U
  - **DONE** Figure out wires and add them
  - **DONE** Add TX23U to probe command
  - **DONE** Add TX23U to sampling cycle
- LORA support
  - **DONE** See if I can put wires on modules. I can.
  - Wire up radio
  - Make a receiver to USB stdio logger
  - Choose modulation parameters
  - Choose payload format
- Build a dedicated receiver
  - Choose meters/displays
  - Wire up
  - Code
