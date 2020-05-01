

// https://pmdway.com/products/0-96-80-x-160-full-color-lcd-module
#include <UTFT.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <EEPROM.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();



// Define buttons
//
#define BUTTON_A  3
#define UNITS 2




// Library only supports software SPI at this time
//NOTE: support  DUE , MEGA , UNO
//SDI=11  SCL=13  /CS =10  /RST=8  D/C=9
UTFT myGLCD(ST7735S_4L_80160, 11, 13, 10, 8, 9); //LCD:  4Line  serial interface      SDI  SCL  /CS  /RST  D/C    NOTE:Only support  DUE   MEGA  UNO
extern uint8_t SmallFont[];
extern uint8_t BigFont[];



int tick;
double emissivity = 0.96;
const double E_MIN = 0.8;
const double E_MAX = 1.0;
const double E_STEP = 0.01;
double ambient, ambientC;
double temper, temperC;
char s[32];
double dt;
char *units;



void setup()
{
  uint16_t ereg;
  double ee;
  unsigned int setup = 0;
  float de;

  Serial.begin(115200);

  // Setup switches and blinky
  //
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(UNITS, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize the sensor
  //
  mlx.begin();
  delay(200);


  // read e here as a check...
  //
  ereg = mlx.readEmissivityReg();
  Serial.println(ereg, HEX);
  ee = mlx.readEmissivity();
  Serial.println(ee);


  // Setup the LCD
  //
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);


  // SETUP Mode
  // Hold the button down on powerup to enter SETUP mode.
  // Units switch determines increment or decrement on each button push.
  // 20 second idle time to get out of setup mode
  //
#define SETUP_DONE 2000
  if (!digitalRead(BUTTON_A)) {

    // Clear the screen and draw the frame
    myGLCD.clrScr();
    myGLCD.setColor(VGA_BLUE);
    myGLCD.fillRect(0, 0, 159, 13);

    myGLCD.setColor(VGA_YELLOW);
    myGLCD.setBackColor(VGA_BLUE);
    myGLCD.print("SETUP Emissitivity", CENTER, 1);
    myGLCD.setFont(SmallFont);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);

    while (setup < SETUP_DONE) {

      de = emissivity + 0.005;
      sprintf(s, "e=%d.%02d ", int(de), int(de * 100) % 100);
      myGLCD.print(s, 18, 26);
      Serial.println(s);

      // wait for button up
      //
      while (!digitalRead(BUTTON_A)) {
        delay(10);
      }
      delay(10); // debounce

      // wait for the button to go down or timeout of out setup mode
      //
      setup = 0;
      while (digitalRead(BUTTON_A)) {
        if (++setup > SETUP_DONE) {
          break;
        }
        delay(10);
      }
      delay(10); // debounce

      // if the button is down then increment/decrement
      //
      if (!digitalRead(BUTTON_A)) {
        if (digitalRead(UNITS)) {
          emissivity += E_STEP;
          if (emissivity > E_MAX) {
            emissivity = E_MAX;
          }
        }
        else {
          emissivity -= E_STEP;
          if (emissivity < E_MIN) {
            emissivity = E_MIN;
          }
        }
      }
    }
    // WRITE E HERE
    //
    mlx.writeEmissivity(emissivity);
  }



  // setup the screen for action
  //
  // Clear the screen and draw the frame
  //
  myGLCD.clrScr();
  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillRect(0, 0, 159, 13);
  myGLCD.setColor(VGA_FUCHSIA);  // ??
  myGLCD.fillRect(0, 114, 159, 127);
  myGLCD.setColor(VGA_YELLOW);
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.print("MakeIt Labs", CENTER, 1);


  temperC = mlx.readObjectTempC();
  delay(2000);
}


double CtoF(double c)
{
  return (c * 9) / 5 + 32;
}



void loop()
{
  if (++tick % 2 == 0) {

    // Only update temperature when button pressed
    //
    if (!digitalRead(BUTTON_A)) {
      temperC = mlx.readObjectTempC();
    }
    ambientC = mlx.readAmbientTempC();

    if (digitalRead(UNITS)) {
      ambient = CtoF(ambientC);
      temper = CtoF(temperC);
      units = "F";
    }
    else {
      ambient = ambientC;
      temper = temperC;
      units = "C";
    }


    myGLCD.setFont(SmallFont);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_BLACK);

    dt = ambient + 0.005;
    sprintf(s, "Ambient %d.%02d %s ", int(dt), int(dt * 100) % 100, units);
    myGLCD.print(s, 18, 26);
    Serial.print(s);

    // Determine the color based on fever.  Temperatures are arbitrarily based on google.
    //
    myGLCD.setFont(BigFont);
    if (temperC > 39.0) {
      myGLCD.setColor(VGA_RED);
    }
    else if (temperC > 38) {
      myGLCD.setColor(VGA_YELLOW);
    }
    else {
      myGLCD.setColor(VGA_GREEN);
    }

    dt = temper + 0.005;
    sprintf(s, " %d.%02d %s ", int(dt), int(dt * 100) % 100, units);
    myGLCD.print(s, 16, 52);
    Serial.println(s);
  }

  if (tick & 4) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(200);
}
