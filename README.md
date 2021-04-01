AT24C32# sbi-weather
My Raspberry Pi Pico weather station

## Salient Points

- There are many of these, this one is mine.
- It is my first pico project.
- It will use LORA to speak with an intermittently available gateway
  and store the observed data until it can be sent.
- Windspeed, direction, temperature, humidity, and pressure. I probably
  won't do rain, they just get plugged up in my environment.
- Build host is a Raspberry Pi running Raspbian. Plain old tools, nothing fancy.
- You can plug a console into the USB to configure and interrogate it.

## Peripherals

- BME280 pressure, humidity, temperature. The temperature accuracy isn't great.
  and is subject to self-heating.
- Anemometer and direction froma LaCrosse TX23U wind sensor. This will need
  a PIO state machine to record the bit stream timing for decoding.
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

### DHT11

- Pin is GP22

## TODO

- **DONE** Add a 'time 2021-01-01 12:34:56' command to set time in rtc
- **DONE** Make set RTC also set DS3231 (blocking is ok)
- RTC returns bad time on first read in `time` command.
- Make the sampling cycle
  - **DONE** Function to get DS3231 into datetime_t (blocking is ok)
  - **DONE** Load RTC from DS3231 on boot
  - Use RTC alarm to trigger sampling cycle (set a flag for main loop)
  - Add DHT11 scan to sampling cycle (blocking for now until I write PIO)
  - Add DS3231 temperature to sampling cycle
  - Pull in BME280 decoding from someone else
  - Add BME280 decoding to sampling cycle (blocking for now)
- Support TX23U
  - Figure out wires and add them
  - Add TX23U to probe command
  - Add TX23U to sampling cycle
- LORA support
  - See if I can put wires on modules, or order bigger ones
  - Wire up radio
  - Make a receiver to USB stdio logger
  - Choose modulation parameters
  - Choose payload format
- Build a dedicated receiver
  - Choose meters/displays
  - Wire up
  - Code
