#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define PIN_SOILMOISTURESENSOR 34

//my soil calibration constants (esp32 12-bit ADC)
const int soilHigherValue = 3500;
const int soilLowerValue  = 1650;

const int samplingCount = 50;
const int samplingTolerance = 100;

LiquidCrystal_I2C lcd (0x27, 16,2);

long getSoilMoistureSamplingAverage();

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  long reading = getSoilMoistureSamplingAverage();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String(reading) + "%"); 
}

long getSoilMoistureSamplingAverage()
{
  int sampling[samplingCount];
  int readingIndex = 0;
  int readingTotal = 0;
  int readingFaultsCount = 0;
  long soilMoisturePercentage = 0;

  //read all samples
  while (readingIndex < samplingCount) {
    sampling[readingIndex++] = analogRead(PIN_SOILMOISTURESENSOR);
    delay(1000/samplingCount);
  }

  //calculate average
  for (byte samplesIndex = 0; samplesIndex < samplingCount; samplesIndex++)
    readingTotal += sampling[samplesIndex];
  long average = readingTotal / (samplingCount);

  //recalculate average without the fault values (beyond the allowed tolerance)
  readingTotal = 0;
  for (byte samplesIndex = 0; samplesIndex < samplingCount; samplesIndex++)
    if(sampling[samplesIndex] > average - samplingTolerance || sampling[samplesIndex] < average + samplingTolerance)
      readingTotal += sampling[samplesIndex];
    else
      readingFaultsCount++;
  average = readingTotal / (samplingCount - readingFaultsCount);
  
  //map and cap average to percentages
  soilMoisturePercentage = constrain(map(average, soilHigherValue, soilLowerValue, 0, 100), 0, 100);

  return soilMoisturePercentage;
}