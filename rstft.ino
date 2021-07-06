// rstft : Radio Shack TFT display

/* electronic hardware
 *   Arduino Uno
 *   NTC10k thermistor with 10K resistor
 *   LDR
 *   DS3231 real-time clock
 *   Seeed Studio 2.8" TFT TouchShield v1.0
 */

// TODO: TFT

/* 2.8" TFT TouchShield v1.0
 * www.seeedstudio.com
 * purchased on stock closeout from Radio Shack
 * NOT a current model so use the special support from .zip file
 * uses pins A0-A3 & D2-D13
 */

/* LDR wiring:
 * 
 * Vcc --- R_55k --- ADC --- LDR --- GND
 */

/* NTC10K wiring:
 *  
 * Vcc --- NTC10K --- ADC --- R_10k --- GND
 * 
 * https://www.giangrandi.ch/electronics/ntc/ntc.shtml
 * https://www.digikey.sk/en/maker/projects/how-to-measure-temperature-with-an-ntc-thermistor/4a4b326095f144029df7f2eca589ca54
 */

/* DS3231 RTC (real time clock)
 * connected by I2C (SDA & SCL)
 * requires library: RTClib
 * https://github.com/adafruit/RTClib
 */

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"

#define LDRPIN A4
#define NTCPIN A5
#define LOOPDELAY 1000
#define LIGHTING_THRESHOLD 850
#define ADC_RESOLUTION (1/1024.)
#define CELSIUS_OFFSET 273.15
#define NTC_T_REF (25 + CELSIUS_OFFSET)
#define NTC_R_REF 10000
#define NTC_BETA 3950

// temperature smoothing (0..1) higher means smoother
#define SMOOTHING 0.7

// actual measured resistance for ntc10k curcuit
#define R_10K  9970

double temperature;
RTC_DS3231 rtc;


void setup() {
  Serial.begin(115200);
  Serial.println("rstft");

  temperature = ntc10k(NTCPIN);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}


void loop() {
  int ldr, ntc;
  DateTime now = rtc.now();

  ldr = analogRead(LDRPIN);
  double sig;

  sig = ntc10k(NTCPIN);
  temperature = temperature*SMOOTHING + sig*(1-SMOOTHING);

  Serial.print(now.unixtime());

  Serial.print(" LDR=");
  Serial.print(ldr);
  if (ldr < LIGHTING_THRESHOLD) {
    Serial.print(" (ON)");
  } else {
    Serial.print(" (OFF)");
  }

  Serial.print(" NTC10K=");
  Serial.print(analogRead(NTCPIN));
  Serial.print(" T=");
  Serial.print(temperature, 3);

  Serial.print(" RTC_T=");
  Serial.print(1.8*rtc.getTemperature()+32, 2);

  Serial.print(" ");
  Serial.print(now.timestamp(DateTime::TIMESTAMP_DATE));
  Serial.print(" ");
  Serial.print(now.timestamp(DateTime::TIMESTAMP_TIME));

  Serial.println("");

  delay(LOOPDELAY);
}


/* ntc10k
 *  
 * get reading from ADC pin with NTC10K voltage
 * 
 * valid range: 100 < adc < 900
 * TODO: How to handle out of range?
 */
double ntc10k(int pin) {
  int adc;
  double v;  // reduced voltage: V/Vcc
  double r;  // resistance, Ohms
  double kelvin;  // absolute temperature
  double fahrenheit;  // temperature

  adc = analogRead(pin);
  v = adc * ADC_RESOLUTION;
  Serial.print(" V=");
  Serial.print(v*5,3);
  r = R_10K*(1/v - 1);
  // r = R_10K*(1/(1-v) - 1);
  Serial.print(" R=");
  Serial.print(r,0);
  Serial.print("  ");
  kelvin = 1/(log(r/NTC_R_REF)/NTC_BETA + 1/NTC_T_REF);
  fahrenheit = 1.8*(kelvin - CELSIUS_OFFSET) + 32;

  return(fahrenheit);
}
