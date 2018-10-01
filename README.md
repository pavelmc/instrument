# RF multi-instrument #

This is my very cheap try to make a RF multi-instrument with as much uses I can pack on it.

I insist: this is as cheap & accurate as I can get from parts in my junk box; yet its a quite usable instrument,
no NIST precision but quite ball park values that is enough for ham tech.

## Features ##

So far I can do all of this things with it:

* Very accurate signal generator (down to a couple of Hz if your calibration source is accurate enough)
* Sweep analyzer to measure filters (low-pass, high-pass, band-pass, stop-band, notch), crystals filters, cristals and many others.
* Spectrum analyzer to check mixers symmetry, and other things.
* Power meter for levels between about a few mV to about ~5.0v pep. (readings in mV, dBm, and power in mW)
* Very low inductance meter at work frequency (as low as 5 nH, theoretically down to 1nH)
* Computer control via USB serial, with a custom Python & pyQT5 program.
* Settings for tune and adjust the instrument.

As usual, imagination and clever ideas can expand the use of this instrument. For example in sweep analyzer mode you can put a SWR bridge with a proper balum and measure the SWR sweet spot of an antenna.

## Technical details ##

* Freq range from 100 KHz to ~200 Mhz in 1Hz steps.
* Receiver and detector has a limited dynamic range of about ~40 dB below maximum signal (no signal is fixed at about -39 to -41 dBm, max signal is taken to 0 dBm or a close value)
* Receiver is made of discrete parts, and it has a 1st IF of ~27 MHz and 2nd If of about 5 kHz (yes, audio IF)
* First IF uses a two 27 Mhz crystals matched with the instrument itself as a narrow (~1 kHz) filter.
* Detector is made of a very efficient AD-DC full wave rectifier with OpAmps (aka: a super detector, that's why the 2nd IF in audio)
* Sweep in the hardware is made with 320 points on each range (the width of the TFT screen) in the PC mode you can use up to 10k points
* This circuit uses a negative power source to feed the OpAmps to get them work lineally near or in the zero range.
* The sketch uses a cheap SPI flash of 4MBytes as a RAM buffer (W25Q32) to store readings before the display can show them, some tricks are implemented to fight wear out of the SPI and to use them as RAM.

No schematics so far as it evolves continually, I plan to switch from the 2 If and super detector model to a model with just one IF and a AD8307, in the future.

### Arduino sketch ###

You will need the following libs to compile it properly

* Si5351mcu: https://github.com/pavelmc/si5351mcu/
* BMux: https://github.com/pavelmc/BMux/
* Adafruit_GFX: https://github.com/adafruit/Adafruit-GFX-Library/
* Adafruit_ILI9340: https://github.com/adafruit/Adafruit_ILI9340/
* SPIFlash: https://github.com/Marzogh/SPIFlash/
* Rotary: // https://github.com/mathertel/RotaryEncoder/
* Bounce2: // https://github.com/thomasfredericks/Bounce2/

# Check again later for updates #
