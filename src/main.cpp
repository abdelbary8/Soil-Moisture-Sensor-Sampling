#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define PIN_SOILMISTURESENSOR 34
#define PIN_PUMP 23


//my soil calibration constant based on the esp32 12-bit ADC
const int soilHigherValue = 3500;
const int soilLowerValue  = 1650;
const long SoilMoistureThreshold = 20; //%

const int samplingCount = 50;
const int samplingTolerance = 100;

LiquidCrystal_I2C lcd (0x27, 16,2);

long getSoilMoistureSamplingAverage();
void powerPumpOnFor(int);

void setup() {
  pinMode(PIN_PUMP, OUTPUT);

  lcd.init();
  lcd.backlight();
}

void loop() {
  long soilReadingAverage = getSoilMoistureSamplingAverage();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String(soilReadingAverage) + "%"); 

  if(soilReadingAverage <= SoilMoistureThreshold){
    lcd.setCursor(0, 1);
    lcd.print("Pump On");

    powerPumpOnFor(1000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(soilReadingAverage) + "%"); 
    lcd.setCursor(0, 1);
    lcd.print("Watering Done");

    delay(10000); //wait for 10 seconds
  }
  

}

long getSoilMoistureSamplingAverage()
{
  int sampling[samplingCount];
  int soilReadingAverageIndex = 0;
  int soilReadingAverageTotal = 0;
  int soilReadingAverageFaultsCount = 0;
  long soilMoisturePercentage = 0;

  //read all samples
  while (soilReadingAverageIndex < samplingCount) {
    sampling[soilReadingAverageIndex++] = analogRead(PIN_SOILMISTURESENSOR);
    delay(1000/samplingCount);
  }

  //calculate average
  for (byte samplesIndex = 0; samplesIndex < samplingCount; samplesIndex++)
    soilReadingAverageTotal += sampling[samplesIndex];
  long average = soilReadingAverageTotal / (samplingCount);

  //recalculate average without the fault values (beyond the allowed tolerance)
  soilReadingAverageTotal = 0;
  for (byte samplesIndex = 0; samplesIndex < samplingCount; samplesIndex++)
    if(sampling[samplesIndex] > average - samplingTolerance || sampling[samplesIndex] < average + samplingTolerance)
      soilReadingAverageTotal += sampling[samplesIndex];
    else
      soilReadingAverageFaultsCount++;
  average = soilReadingAverageTotal / (samplingCount - soilReadingAverageFaultsCount);
  
  //map and cap average to percentages
  soilMoisturePercentage = constrain(map(average, soilHigherValue, soilLowerValue, 0, 100), 0, 100);

  return soilMoisturePercentage;
}

void powerPumpOnFor(int milliSeconds){
  analogWrite(PIN_PUMP, 150);
  delay(milliSeconds);
  analogWrite(PIN_PUMP, 0);
}