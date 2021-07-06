// rstft : Radio Shack TFT display

/* electronic hardware
 *   Arduino Uno
 *   NTC10k thermistor with 10K resistor
 *   LDR
 *   DS3231 real-time clock, I2C
 *   Seeed Studio 2.8" TFT TouchShield v1.0
 *   ADS1115 4-ch 16-bit A/D converter, I2C
 */

// TODO: TFT

/* 2.8" TFT TouchShield v1.0
 * www.seeedstudio.com
 * purchased on stock closeout from Radio Shack
 * NOT a current model so use the special support from .zip file
 * uses pins A0-A3 & D2-D13
 */

// NOTE: Uno uses A4 & A5 for I2C interface!

/* LDR wiring: ADS1115 pin A2
 * 
 * Vcc --- R_55k --- ADC --- LDR --- GND
 */

/* NTC10K wiring: ADS1115 pin A3
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

// ADS1115 A/D converter connected via I2C and Wire lib
#include<ADS1115_WE.h> 
#include<Wire.h>

#define ADS1115_I2C_ADDRESS 0x48

#define LOOPDELAY 1000
#define LIGHTING_THRESHOLD 3.01
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

ADS1115_WE ads1115 = ADS1115_WE(ADS1115_I2C_ADDRESS);
bool first_read = true;


void setup() {
  Serial.begin(115200);
  Serial.println("rstft");

  Wire.begin();

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
  
  if(!ads1115.init()){
    Serial.println("ADS1115 not connected!");
    abort();
  }
  ads1115.setVoltageRange_mV(ADS1115_RANGE_6144); // +/- 6144 mVDC
  ads1115.setCompareChannels(ADS1115_COMP_0_GND); // single-ended
  ads1115.setCompareChannels(ADS1115_COMP_1_GND); // single-ended
  ads1115.setCompareChannels(ADS1115_COMP_2_GND); // single-ended
  ads1115.setCompareChannels(ADS1115_COMP_3_GND); // single-ended
  // ads1115.setConvRate(ADS1115_8_SPS); // the default
  ads1115.setMeasureMode(ADS1115_CONTINUOUS); //default is ADS1115_SINGLE
  // ALERT pin is not wired so ignore its configuration
}


void loop() {
  int ldr, ntc;
  DateTime now = rtc.now();

  ldr = readADS115Channel(ads1115, ADS1115_COMP_2_GND);

  double sig = ntc10k();
  if (first_read) {
    temperature = sig;
  } else {
    temperature = temperature*SMOOTHING + sig*(1-SMOOTHING);
  }

  Serial.print(now.unixtime());

//  Serial.print(" LDR=");
//  Serial.print(ldr, 3);
  if (ldr < LIGHTING_THRESHOLD) {
    Serial.print(" (ON)");
  } else {
    Serial.print(" (OFF)");
  }

  Serial.print(" T=");
  Serial.print(temperature, 3);

//  Serial.print(" A2=");
//  Serial.print(readADS115Channel(ads1115, ADS1115_COMP_2_GND), 3);
//
//  Serial.print(" A3=");
//  Serial.print(readADS115Channel(ads1115, ADS1115_COMP_3_GND), 3);

  Serial.print(" RTC_T=");
  Serial.print(1.8*rtc.getTemperature()+32, 1);

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
double ntc10k() {
  int adc;
  double v;  // reduced voltage: V/Vcc
  double r;  // resistance, Ohms
  double kelvin;  // absolute temperature
  double fahrenheit;  // temperature

  v = readADS115Channel(ads1115, ADS1115_COMP_3_GND) / 5;
  r = R_10K*(1/v - 1);

//  Serial.print(" V=");
//  Serial.print(v, 3);
//  Serial.print(" R=");
//  Serial.print(r, 0);
//  Serial.print("  ");

  kelvin = 1/(log(r/NTC_R_REF)/NTC_BETA + 1/NTC_T_REF);
  fahrenheit = 1.8*(kelvin - CELSIUS_OFFSET) + 32;

  return(fahrenheit);
}

float readADS115Channel(ADS1115_WE adc, ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
  return voltage;
}
