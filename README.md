# temperbox
MLX90614 based non contact thermometer with .96" color TFT and Emissivity adjustment

I don't have a fritzing but pretty simple.  I used a Metro Mini which is a small format Uno.

Sensor is I2C on pins A4, A5 for SDA/SCL.

The .96" TFT uses SPI on pins as shown in the code with the library referenced.

I use Pin 2 for the measurement Mode and  Units switch (C/F) and Pin 3 for the Hold/Sample button.

In order to change the Emissivity factor in the sensor I needed to add that feature to the Adafruit library.  If those functions are not in arduino library manager you will need to install it yourself from Github: https://github.com/adafruit/Adafruit-MLX90614-Library

Normal operation  powers up in a couple seconds and shows sensor ambient temperature and an initial sample of the measured temperature.  Hold the button down to read a few samples per second.  The last reading when the button is released it held.  You can flip the units switch any time to change units.

To change between C and F units short press the sample button.

The sensor has a register for setting the Emissivity factor.  This tells the sensor how close to an ideal "black body" radiator you are measuring.  Different materials radiate heat differently.  The temperbox has a setup mode for this if you power up holding the button down.  Pressing the button will increment or decrement the factor depending on the units swtich.  When done setting wait for about 20 seconds timout and the value will be written to the EEPROM of the sensor and normal operation will continue.

NOTES On use as a Fever Thermometer

I put this together to make a fever thermometer since they are difficult to purchase.  One thing I found is that normal forehead temperature seems to read in the low 90 degrees F.  Oh, it must be the E factor!  Well, skin is actually about .96 or higher and not much of a change in the final reading.  Some sites suggest a factor of .86 which helps but doesn't get close to 98 decgrees or what we expect "normal" healthy himan temperature.   

I bought a "real" thermometer from Banggood and the manual had the conversion chart between actual measurement and what it displayed for the fever temperature.  I changed the code to do a linear interpolation of this chart and also added a fudge factor "calibration" like the Banggood one did but I don't have a UI to change it yet.  Todo was also add it to EEPROM.


