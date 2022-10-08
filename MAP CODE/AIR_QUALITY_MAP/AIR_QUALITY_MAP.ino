
#include <FastLED.h>

// 0 >> AIR QUALITY BAR
// 1 >> ETI-OSA
// 2 >> BADAGRY CREEK
// 3 >> LAGOS ISLAND
// 4 >> APAPA
// 5 >> AJENGUNLE
// 6 >> OJO
// 7 >> LAGOS MAINLAND
// 8 >> AMUWO ODOFIN
// 9 >> SURULERE
// 10 >> OSHODI ISOLO
// 11 >> ALIMOSHO
// 12 >> SHOMOLU
// 13 >> MUSHIN
// 14 >> KOSOFE
// 15 >> AGEGE
// 16 >> IKEJA
// 17 >> IKORODU
// 18 >> IFAKO IJAYE

#define co2 1
#define co2_upperlim 5000.0f
#define voc 2
#define voc_upperlim 200.0f
int activeAirQdataPT = 1;
uint8_t dataPTcount = 2;

String receivedsensordata = "";
int currentLoggerLocation = 0;

struct
{
  //***************************//
  String Area_Name = ""; // cell name
  byte startindex = 0;   // led start index
  byte endindex = 20;    // led end index
  int Cap_Signal = 0;    // received captouch signal
  int prevCap_Signal = 0;
  int Cap_Threshold = 250; // touch detection threshold
  float alpha = 0.05;
  int AIRQ_dataPT = 2500;
  int allDataPT[2];
} MAPCELL[19];

// fastLED setup*******************************************************/
#define DATA_PIN 8
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 193
CRGB leds[NUM_LEDS];
#define BRIGHTNESS 25
CRGBPalette16 currentPalette;
TBlendType currentBlending;
uint8_t gHue = 0; // rotating "base color"

struct
{
  uint8_t brightness = BRIGHTNESS;
  uint8_t gHue = 0;
  long gHuetiming = 0;
  long timing = 0;
  uint8_t state = 0;
} Dleds[NUM_LEDS];

struct
{
  int beatcount = 0;
  int breathfader = 1;
} A_variables[NUM_LEDS];

int blackpoint = 240;
/*********************************************************************************/

/***************AIR Q LEVEL BAR*************************/
long barcolors[13] = {
    CRGB::White,
    CRGB::Red,
    CRGB::OrangeRed,
    CRGB::Tomato,
    CRGB::Coral,
    CRGB::DarkOrange,
    CRGB::Orange,
    CRGB::Yellow,
    CRGB::GreenYellow,
    CRGB::PaleGreen,
    CRGB::Lime,
    CRGB::DarkTurquoise,
    CRGB::DeepSkyBlue};
/*********************************************************/

String inString, csignal, cindex;
byte inByte;

int captouchprocesstimer = 50;
long captouchprocesstimertracker = 0;

void setupMapCellVariables()
{
  MAPCELL[0].Area_Name = "AIRQ_BAR";
  MAPCELL[0].startindex = 0;
  MAPCELL[0].endindex = 12;

  MAPCELL[1].Area_Name = "Eti-osa";
  MAPCELL[1].startindex = 13;
  MAPCELL[1].endindex = 30;

  MAPCELL[2].Area_Name = "Badagry Creek";
  MAPCELL[2].startindex = 31;
  MAPCELL[2].endindex = 57;

  MAPCELL[3].Area_Name = "Lagos-island";
  MAPCELL[3].startindex = 58;
  MAPCELL[3].endindex = 62;

  MAPCELL[4].Area_Name = "Apapa";
  MAPCELL[4].startindex = 63;
  MAPCELL[4].endindex = 66;

  MAPCELL[5].Area_Name = "Ajengunle";
  MAPCELL[5].startindex = 67;
  MAPCELL[5].endindex = 70;

  MAPCELL[6].Area_Name = "Ojo";
  MAPCELL[6].startindex = 71;
  MAPCELL[6].endindex = 88;

  MAPCELL[7].Area_Name = "Lagos-Mainland";
  MAPCELL[7].startindex = 89;
  MAPCELL[7].endindex = 93;

  MAPCELL[8].Area_Name = "Amuwo-odofin";
  MAPCELL[8].startindex = 94;
  MAPCELL[8].endindex = 105;

  MAPCELL[9].Area_Name = "Surulere";
  MAPCELL[9].startindex = 106;
  MAPCELL[9].endindex = 108;

  MAPCELL[10].Area_Name = "Oshodi/isolo";
  MAPCELL[10].startindex = 109;
  MAPCELL[10].endindex = 116;

  MAPCELL[11].Area_Name = "Alimosho";
  MAPCELL[11].startindex = 117;
  MAPCELL[11].endindex = 138;

  MAPCELL[12].Area_Name = "Shomolu";
  MAPCELL[12].startindex = 139;
  MAPCELL[12].endindex = 141;

  MAPCELL[13].Area_Name = "Mushin";
  MAPCELL[13].startindex = 142;
  MAPCELL[13].endindex = 145;

  MAPCELL[14].Area_Name = "Kosofe";
  MAPCELL[14].startindex = 146;
  MAPCELL[14].endindex = 157;

  MAPCELL[15].Area_Name = "Agege";
  MAPCELL[15].startindex = 158;
  MAPCELL[15].endindex = 163;

  MAPCELL[16].Area_Name = "ikeja";
  MAPCELL[16].startindex = 164;
  MAPCELL[16].endindex = 173;

  MAPCELL[17].Area_Name = "Ikorodu";
  MAPCELL[17].startindex = 174;
  MAPCELL[17].endindex = 184;

  MAPCELL[18].Area_Name = "ifako-ijaye";
  MAPCELL[18].startindex = 185;
  MAPCELL[18].endindex = 192;
}

void setup()
{
  SerialUSB.begin(115200);
  Serial1.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  setupMapCellVariables();
  mapStartup();
}

void loop()
{
  // rainbow(0);
  getCaptouch_SerialUSB();
  processCellProximity();
  // updateAIRQ_Bar(11);
}

void getCaptouch_SerialUSB()
{
  if (Serial1.available() > 0)
  {
    /******************************************/
    inByte = Serial1.read();
    if (inByte != '*')
    {
      inString += (char)inByte;
    }
  }
  if (inByte == '*')
  {
    inByte = 0;
    if (inString != "")
    {
      csignal = inString.substring(3);
      cindex = inString.substring(0, 2);
      int cindex_int = cindex.toInt() + 3;
      MAPCELL[cindex_int].Cap_Signal = int((float(csignal.toInt()) * MAPCELL[cindex_int].alpha) + (float(MAPCELL[cindex_int].prevCap_Signal) * (1 - MAPCELL[cindex_int].alpha)));
      MAPCELL[cindex_int].prevCap_Signal = MAPCELL[cindex_int].Cap_Signal;

      // MAPCELL[cindex.toInt()].Cap_Signal = csignal.toInt();

      //set threshold at startup
      if (MAPCELL[cindex_int].Cap_Threshold == 250)
      {
        MAPCELL[cindex_int].Cap_Threshold = csignal.toInt();
      }
      
      if (cindex_int == 6)
      {
        SerialUSB.println(inString);
      }
      
      
      

      inString = "";
    }
  }
}

void mapStartup()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Gold;
    FastLED.show();
    delay(10);
  }
  for (int i = NUM_LEDS; i > 0; i--)
  {
    leds[i - 1] = CRGB::Black;
    FastLED.show();
    delay(10);
  }
}

void processCellProximity()
{
  if ((millis() - captouchprocesstimertracker) > captouchprocesstimer)
  {
    bool proximitydetected = false;
    // check if any cell is touched
    for (int i = 0; i < 19; i++)
    {
      if (MAPCELL[i].Cap_Signal > MAPCELL[i].Cap_Threshold)
      {
        SerialUSB.print(i);
        SerialUSB.print("\t");
        SerialUSB.print(MAPCELL[i].Cap_Signal);
        SerialUSB.print("\t");
        SerialUSB.println(MAPCELL[i].Cap_Threshold);

        if (!proximitydetected)
        {
          // turn off all map leds
          for (int i = 0; i < NUM_LEDS; i++)
          {
            leds[i] = CRGB::Black;
          }
          proximitydetected = true;
        }
        //  turn on MAPCELLs with proximity
        long col = updateAIRQ_Bar(getAIRQlevel_BAR(MAPCELL[i].AIRQ_dataPT));
        for (int j = MAPCELL[i].startindex; j < MAPCELL[i].endindex; j++)
        {
          leds[j] = col;
           //leds[j] = CRGB::Green;
        }
        MAPCELL[i].Cap_Signal = 0;
      }
    }

    if (proximitydetected)
    {
      FastLED.show();
    }
    else
    {
      // default animation
      // rainbow(MAPCELL[0].endindex + 1);
      rainbow(0);
      FastLED.show();
    }
    
    captouchprocesstimertracker = millis();
  }
}

void rainbow(int ledindex)
{
  // FastLED's built-in rainbow generator
  if ((millis() - Dleds[ledindex].gHuetiming) > 20)
  {
    fill_rainbow(leds, NUM_LEDS, Dleds[ledindex].gHue++, 7);
    if ((millis() - Dleds[ledindex].gHuetiming) > 40)
    {                         // check timing
      Dleds[ledindex].gHue++; // slowly cycle the "base color" through the rainbow
      Dleds[ledindex].gHuetiming = millis();
      leds[ledindex] = ColorFromPalette(currentPalette, Dleds[ledindex].gHue, Dleds[ledindex].brightness, currentBlending);
    }
    Dleds[ledindex].gHuetiming = millis();
  }
}

void changeAIRQdataPT()
{
  if (activeAirQdataPT < dataPTcount)
  {
    activeAirQdataPT += 1;
  }
  else
  {
    activeAirQdataPT = 1;
  }
  for (size_t i = 1; i < sizeof(MAPCELL) / sizeof(MAPCELL[0]); i++)
  {
    MAPCELL[i].AIRQ_dataPT = MAPCELL[i].allDataPT[activeAirQdataPT];
  }
}

int getAIRQlevel_BAR(int airqdata)
{
  if (activeAirQdataPT == co2)
  {
    int aqbarlevel = int((float(airqdata) / co2_upperlim) * float((sizeof(barcolors) / sizeof(barcolors[0])) - 1));
    if (aqbarlevel < 1)
    {
      aqbarlevel = 1;
    }
    // SerialUSB.println(aqbarlevel);
    return aqbarlevel;
  }
  else if (activeAirQdataPT == voc)
  {
    // return (100.0 / voc_upperlim) * float(airqdata);
  }
  return 0;
}

long updateAIRQ_Bar(int airqdatabarlevel)
{
  for (int j = MAPCELL[0].endindex + 1; j > 0; j--)
  {
    if ((sizeof(barcolors) / sizeof(barcolors[0])) - j < airqdatabarlevel)
    {
      leds[j - 1] = barcolors[j - 1];
    }
    else
    {
      if (j == 1) // BAR NAME
      {
        leds[j - 1] = barcolors[j - 1];
      }
      else
      {
        leds[j - 1] = CRGB::Black;
      }
    }
  }
  //FastLED.show();
  return barcolors[(sizeof(barcolors) / sizeof(barcolors[0])) - airqdatabarlevel];
}

String getactiveLocationName()
{
  int si = receivedsensordata.lastIndexOf(",") + 1;
  String res = "";
  for (int i = si; i < receivedsensordata.length(); i++)
  {
    if (receivedsensordata[i] != '*')
    {
      res += receivedsensordata[i];
    }
  }
  return res;
}

int getsensorlevel(String sensorID)
{
  int si = receivedsensordata.lastIndexOf(sensorID) + sensorID.length();
  String res = "";
  int maxcheckrange = si + 10;
  for (int i = si; i < maxcheckrange; i++)
  {
    if (receivedsensordata[i] != ':')
    {
      if (receivedsensordata[i] != ',')
      {
        res += receivedsensordata[i];
      }
      else
      {
        i = maxcheckrange;
      }
    }
  }
  return res.toInt();
}

void updateAirQdata()
{
  // get area name
  if (receivedsensordata != "")
  {
    String A_name = getactiveLocationName();
    for (size_t i = 0; i < sizeof(MAPCELL) / sizeof(MAPCELL[0]); i++)
    {
      if (MAPCELL[i].Area_Name == A_name)
      {
        // get all data points
        MAPCELL[i].allDataPT[co2] = getsensorlevel("co2");
        MAPCELL[i].allDataPT[voc] = getsensorlevel("cocIndex");

        // update active sensor data point
        MAPCELL[i].AIRQ_dataPT = MAPCELL[i].allDataPT[activeAirQdataPT];

        // update current Logger Location
        currentLoggerLocation = i;

        receivedsensordata = "";
        break;
      }
    }
  }
}

void animateLoggerLocation()
{
  for (size_t i = 0; i < sizeof(MAPCELL) / sizeof(MAPCELL[0]); i++)
  {
    if (i == currentLoggerLocation)
    {
      // animate logger location (cardio)
    }
  }
}
