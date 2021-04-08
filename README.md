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

- When the watch reboots it will display its IP address, just visit it with a browser.

  Note: no need to set the IP in the web page (unless you opened the html file another way than from the watch)

## Endpoints

- `POST /raw`: expects a single file in `multipart/form-data` format of exactly 5000 (200x200/8) bytes. Copies the file into the display buffer and updates the screen.

    eg:
      `curl -F "data=@buffer.bin" http://$WATCHY_IP/raw`

- `GET /fonts`: get a comma-separated list of font names acceptable to `/drawText`

    eg:
      `curl http://$WATCHY_IP/fonts`

- `POST /drawText`: render text on the watchy and update the display. Expects a `application/x-www-form-urlencoded` body with the following parameters:
    - `f`: font name
    - `t`: text string to display
    - `x` and `y`: coordinates to render at (in that weird Adafruit GFX format - some issues with `y` pointing to the top or baseline)
    - `c`: if non empty and non zero, clear the full screen before drawing the text
    - `i`: if non empty and non zero, invert colours. If inverted, text is rendered in white, and if clear is requested, clears to black.

    eg:
      `curl -d "f=FreeMonoBold9pt7b&t=hello&x=0&y=50&c=1&i=0" http://$WATCHY_IP/drawText`

## License

               DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                       Version 2, December 2004
     
    Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

    Everyone is permitted to copy and distribute verbatim or modified
    copies of this license document, and changing it is allowed as long
    as the name is changed.
     
               DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
      TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

     1. You just DO WHAT THE FUCK YOU WANT TO.
