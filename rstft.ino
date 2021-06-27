// rstft : Radio Shack TFT display

/* electronic hardware
 *   Arduino Uno
 *   NTC10k thermistor with 10K resistor
 *   LDR
 *   DS3231 real-time clock
 *   Seeed Studio 2.8" TFT TouchShield v1.0
 */

// TODO: DS3231
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

#define LDRPIN A0
#define NTCPIN A1
#define VCCPIN A2
#define LOOPDELAY 1000
#define LIGHTING_THRESHOLD 600
#define ADC_RESOLUTION (1/1024.)
#define CELSIUS_OFFSET 273.15
#define NTC_T_REF (25 + CELSIUS_OFFSET)
#define NTC_R_REF 10000
#define NTC_BETA 3950
#define SMOOTHING 0.7

// actual measured resistance for ntc10k curcuit
#define R_10K  9810

double temperature;


void setup() {
  Serial.begin(9600);
  Serial.println("rstft");

  temperature = ntc10k(NTCPIN);
}


void loop() {
  int ldr, ntc;

  ldr = analogRead(LDRPIN);
  double sig;

  Serial.print(" LDR=");
  Serial.print(ldr);
  if (ldr < LIGHTING_THRESHOLD) {
    Serial.print(" (lights ON)");
  } else {
    Serial.print(" (lights OFF)");
  }

  sig = ntc10k(NTCPIN);
  temperature = temperature*SMOOTHING + sig*(1-SMOOTHING);
  Serial.print(" fahrenheit=");
  Serial.print(temperature, 3);

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
  r = R_10K*(1/v - 1);
  kelvin = 1/(log(r/NTC_R_REF)/NTC_BETA + 1/NTC_T_REF);
  fahrenheit = 1.8*(kelvin - CELSIUS_OFFSET) + 32;

  return(fahrenheit);
}
