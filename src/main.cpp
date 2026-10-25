/* AS7341 spectrometer for CYD */

// Channel = the channel of the multi spectral sensor
// Wavelenght = the calculated wavelenght from the calibration matrix and channels data
// Band = the values shown on the display

#include <Adafruit_AS7341.h>
#include <TFT_eSPI.h>       // Include the graphics library

#include "matrixdata.h"

#define I2C_SDA 22
#define I2C_SCL 27

#define FONT_SIZE 2
#define BG_COLOR TFT_BLACK
#define GRAPH_HEIGHT 220
#define BAND_SKIP 24

#define ATIME 5
#define ASTEP 3596
#define ADC_GAIN AS7341_GAIN_128X

#define PLOT_SCALE 10000

#define LUX_COEFFICIENT 683

TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

TwoWire I2CAS7341 = TwoWire(0); //I2C pin set https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/#1
Adafruit_AS7341 as7341;


constexpr int bandStep = TFT_HEIGHT / channelsAmount;
constexpr int bandCenter = bandStep / 2;
constexpr int UIheight = TFT_WIDTH - GRAPH_HEIGHT;

double basicCounts[channelsAmount];
double testCounts[channelsAmount];
int testCounter = 0;

uint16_t readings[12];

struct SensorBands
{
  int32_t colorTFT;
  float corrFactor;
  String bandName;
};

SensorBands sensorbands[] = {
  {TFT_VIOLET, 3.20, "415"}, //F1
  {TFT_BLUE, 3.00, "445"}, //F2
  {TFT_CYAN, 2.07, "480"}, //F3
  {TFT_GREEN, 1.30, "515"}, //F4
  {TFT_GREENYELLOW, 1.07, "555"}, //F5
  {TFT_YELLOW, 0.93, "590"}, //F6
  {TFT_ORANGE, 0.78, "630"}, //F7
  {TFT_RED, 0.71, "680"}, //F8
  {TFT_WHITE, 1, "CL"}, //Clear
  {TFT_MAROON, 1, "IR"} //NIR
};

void as7341setup() {
  I2CAS7341.begin(I2C_SDA, I2C_SCL, 100000); //I2C pin set
  
  as7341.begin(0x39, &I2CAS7341);
  
  as7341.setATIME(ATIME);
  as7341.setASTEP(ASTEP);
  as7341.setGain(ADC_GAIN);
}

void TFTsetup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG_COLOR);
  tft.setTextFont(FONT_SIZE);
}

void drawUI() {
  int textXPos = 0;
  int bandCounter = 0;

  for (int i = 0; i < TFT_HEIGHT; i++){

    if (bandCounter == 0)
    {
      tft.drawNumber(spectralcorrmatrix[i].wavelenghtNm, i, 0);
      ++bandCounter;
    } else if (bandCounter > BAND_SKIP){
      bandCounter = 0;
    } else {
      ++bandCounter;
    }
  }

  tft.drawFastHLine(0, UIheight - 1, TFT_HEIGHT, TFT_WHITE);
}

void generateTestCounts() {

  for (size_t i = 0; i < channelsAmount; i++)
  {
    testCounts[i] = 0;
  }

  testCounts[testCounter] = 0.1;
  
  if (testCounter < channelsAmount)
  {
    ++testCounter;
  } else {
    testCounter = 0;
  }

  delay(500);
}

void getCounts() {

  for (int i = 0; i < channelsAmount; i++)
  {
    int currentChannelSkip;
      if (i > 3) // Skip repeated channels 4&5
      {
        currentChannelSkip = i + 2;
      } else {
        currentChannelSkip = i;
      }
    basicCounts[i] = as7341.toBasicCounts(readings[currentChannelSkip]);
  }
}

void drawSpectrum() { // Draw the matrix-corrected spectrum to the display

  int rectXPos = 0; // Start drawing from here

  for (int currentWavelenght = 0; currentWavelenght < TFT_HEIGHT; currentWavelenght++) // Loop through wavelenghts and draw them to the display
  {
    double wavelenghtValue = 0;

    for (int currentChannel = 0; currentChannel < channelsAmount; currentChannel++) {// Loop through the sensor channels

      double channelTimesK = basicCounts[currentChannel] * spectralcorrmatrix[currentWavelenght].coefficient[currentChannel]; // Multiply the channel value by the respective coefficient (Matrix Y)
      wavelenghtValue += channelTimesK; // Sum the corrected channels values to a wavelenght value (Matrix X)
    }

    int plotValue = wavelenghtValue * PLOT_SCALE;

    int bandHeight;
    
    if (plotValue > GRAPH_HEIGHT)
    {
      bandHeight = GRAPH_HEIGHT;
    } else {
      bandHeight = plotValue;
    }
    

    tft.drawFastVLine(rectXPos, (TFT_WIDTH - bandHeight), TFT_WIDTH, TFT_WHITE);
    tft.drawFastVLine(rectXPos, UIheight, GRAPH_HEIGHT - bandHeight, BG_COLOR);

    ++rectXPos;
  }
}

void drawBarGraph() {
  int rectXPos = 0;

  for (int i = 0; i < channelsAmount; i++)
  {
    int selectedBand;

    if (i > 3)
    {
      selectedBand = i + 2;
    } else {
      selectedBand = i;
    }

    uint16_t bandCorrected = readings[selectedBand] * sensorbands[i].corrFactor;
    
    int bandHeight = map(bandCorrected, 0, 65536, 0, GRAPH_HEIGHT);

    tft.fillRect(rectXPos, (TFT_WIDTH - bandHeight), bandStep, TFT_WIDTH, sensorbands[i].colorTFT);
    tft.fillRect(rectXPos, UIheight, bandStep, GRAPH_HEIGHT - bandHeight, BG_COLOR);
    rectXPos = rectXPos + bandStep;
  }
}

void drawXYZcoordinates() {

  for (int currentAxis = 0; currentAxis < 3; currentAxis++) // Loop through coordinates axis and draw them to the display
  {
    double axisValue = 0;

    for (int currentChannel = 0; currentChannel < channelsAmount; currentChannel++) {// Loop through the sensor channels

      double channelTimesK = basicCounts[currentChannel] * xyzmatrix[currentAxis].coefficient[currentChannel]; // Multiply the channel value by the respective coefficient (Matrix Y)
      axisValue += channelTimesK; // Sum the corrected channels values to an axis value (Matrix X)
    }

    int XYZ_YPos = (20 + (10 * (currentAxis * 2)));

    tft.drawChar(xyzmatrix[currentAxis].axisChar, 0, XYZ_YPos);
    tft.drawFloat(axisValue, 5, 20 , XYZ_YPos);

    //Draw Lux value
    if (currentAxis == 1)
    {
      int luxValue = axisValue * LUX_COEFFICIENT;
      tft.drawString("Lx", 0, 80);
      tft.drawNumber(luxValue, 20 , 80);
    }
    
  }
}

void setup() {
  as7341setup();
  TFTsetup();
  drawUI();
}

void loop() {
  as7341.readAllChannels(readings);
  //generateTestCounts();
  getCounts();
  //drawBarGraph();
  drawSpectrum();
  drawXYZcoordinates();
}
