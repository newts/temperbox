

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
#define BUTTON_A 3
#define SWITCH_B 2

// todo: UI for fudge factor (sorry I mean user calibration)
//



// Library only supports software SPI at this time
//NOTE: support  DUE , MEGA , UNO
//SDI=11  SCL=13  /CS =10  /RST=8  D/C=9
UTFT myGLCD(ST7735S_4L_80160, 11, 13, 10, 8, 9); //LCD:  4Line  serial interface      SDI  SCL  /CS  /RST  D/C    NOTE:Only support  DUE   MEGA  UNO
extern uint8_t SmallFont[];
extern uint8_t BigFont[];



const double Forehead[] = {
  34.0, 35.0, 35.6, 35.8, 36.0, 36.2, 36.4, 37.0,
};
const double Body[] = {
  36.2, 37.0, 37.5, 37.7, 37.8, 38.0, 38.1, 38.5,
};
const int  NUM_SEGMENTS = ( (sizeof(Body) / (sizeof(double))) - 1);

double BodyFudge = 0.0;
int Farenheight = 0;

double ForeheadToBody(double forehead)
{
  int i = 1;

  // find the right segment
  //
  while (i < NUM_SEGMENTS) {
    if (forehead < Forehead[i]) {
      break;
    }
    i++;
  }
  double m = (Body[i] - Body[i - 1]) / (Forehead[i] - Forehead[i - 1]);
  double b = Body[0] - m * Forehead[0];
  printf("i=%d m=%f b=%f\n", i, m, b);

  return (m * forehead + b) + BodyFudge;
}

int tick, tickdown;
double emissivity = 0.96;
const double E_MIN = 0.8;
const double E_MAX = 1.0;
const double E_STEP = 0.01;
double ambient, ambientC;
double temper, temperC, temperCB;;
char s[64];
double dt;
char *units;
char *mode;



void setup()
{
  uint16_t ereg;
  double ee;
  unsigned int setup = 0;
  float de;

  Serial.begin(115200);
  tickdown = 0;

  // Setup switches and blinky
  //
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(SWITCH_B, INPUT_PULLUP);
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
        if (digitalRead(SWITCH_B)) {
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
  return (c * 9.0) / 5.0 + 32.0;
}



void loop()
{
  if (++tick % 2 == 0) {

    // Only update temperature when button pressed
    //
    if (!digitalRead(BUTTON_A)) {
      if (++tickdown > 1) { // debounce for units change
        temperC = mlx.readObjectTempC();
      }
    }
    else {
      if (tickdown && (tickdown < 2)) {
        Farenheight = !Farenheight; // short press changes units
      }
      tickdown = 0;
    }
    ambientC = mlx.readAmbientTempC();

    if (!digitalRead(SWITCH_B)) {
      mode = "Fever";
      temperCB = ForeheadToBody(temperC);
    }
    else {
      mode = "Actual";
      temperCB = temperC;
    }

    if (Farenheight) {
      ambient = CtoF(ambientC);
      temper = CtoF(temperCB);
      units = "F";
    }
    else {
      ambient = ambientC;
      temper = temperCB;
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

    if (digitalRead(SWITCH_B)) {
      myGLCD.setColor(VGA_BLUE);
    }
    else {
      if (temperCB > 39.4) {
        myGLCD.setColor(VGA_RED);
      }
      else if (temperCB > 37.7) {
        myGLCD.setColor(VGA_YELLOW);
      }
      else {
        myGLCD.setColor(VGA_GREEN);
      }
    }

    dt = temper + 0.005;
    sprintf(s, " %d.%02d %s ", int(dt), int(dt * 100) % 100, units);
    myGLCD.print(s, 4, 52);
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
