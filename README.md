# sbi-weather
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
