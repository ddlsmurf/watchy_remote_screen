# Watchy remote screen

This is a PoC / dev tool for playing with the [Watchy](https://watchy.sqfmi.com/)'s e-ink screen from a browser.

It is not meant as a finished product (very few safety checks, no auth, hacky code, etc...).

## Setup

- Clone this repository
- In `src/main.ino` set `WIFI_SSID` and `WIFI_PASS` to your wifi credentials
- Run `pio run` - it will fail but it will download dependencies
- Patch `.pio/libdeps/esp32dev/GxEPD2/src/GxEPD2_BW.h`, add this to a public section of class GxEPD2_BW:

      uint8_t *getBuffer() {
          return _buffer;
      }

- You may wish to check `platformio.ini` to set your serial port and speed and whatnot.
- Upload the firmware:

      pio run -t upload
      pio run -t uploadfs

- When the watch reboots it will display it's IP address, just visit it with a browser.

  Note: no need to set the IP in the web page (unless you opened the html file another way than from the watch)
