#include <DHT.h>
#include <DHT_U.h>

#include <ILI9341_t3.h>
#include <font_Crystal.h>
#include <font_ArialBlack.h>
#include <font_Michroma.h>
#include <font_AwesomeF000.h>
#include <font_LiberationSansBold.h>

#include <SPI.h>
#include <elapsedMillis.h>

// LCD
#define cs   10  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define dc   9   //  but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
ILI9341_t3 TFTscreen = ILI9341_t3(cs, dc);

int CPUstate = 0;
float touchX;
float touchY;
int x;
int y;

int AMDGPU;
int fanCount;
bool displayReady = false;

// DHT 22
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Color definitions
#define BLACK     0x0000
#define BLUE      0x001F
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define LTBLUE    0x05DF
#define AMDRED    0xF042
#define PURPLE    0x48EC
#define SUNSET    0xFEC0
#define DARKBLUE  0x2896
#define TEAL      0x2551
#define ORANGE    0xFC00
#define LTGREEN   0x47F4

byte setupData[4];
byte inputData[25];
String displayText[25];
elapsedMillis timeElapsed;
elapsedMillis DHTtime;

void setup() {
  TFTscreen.begin();
  TFTscreen.setRotation(1); 
  drawSplash(); 
  dht.begin();
  delay(100);
  int temp = dht.readTemperature();
  int humidity = dht.readHumidity();

  if (temp < 10)
  {
    displayText[12] = "0" + String(temp) + "C";
  }
  else
  {
    displayText[12] = String(temp) + "C";
  }

  if (humidity < 10)
  {
    displayText[13] = "0" + String(humidity) + "C";
  }
  else
  {
    displayText[13] = String(humidity) + "%";
  }

  Serial.begin(115200);
}

void loop() {
  //Read setup buffer
  if (Serial.available() == 4)
  {
    for (int i = 0; i < 4; i++)
    {
      setupData[i] = Serial.read();
    }

    //Detect Command type
    switch (setupData[0])
    {
      case 128:
        {
          //Say hello
          Serial.print("HELLO FROM ARDUINO");
        }
        break;
      case 129:
        {
          drawSplash();
          displayReady = false;
        }
        break;
    }
  }

  else if (Serial.available() == 25)
  {
    // Read the data buffer
    for (int i = 0; i < 25; i++)
    {
      inputData[i] = Serial.read();
    }

    if (displayReady)
    {
      //Detect Command type
      switch (inputData[0])
      {
        case 127:
          {
            // CPU Data
            int fanSpeed = (inputData[2] << 8) | inputData[1];
            displayText[0] = String(fanSpeed) + " RPM  ";
            displayText[1] = String(inputData[3]) + "C  ";
            displayText[2] = String(inputData[4]) + "C  ";
            displayText[3] = String(inputData[5]) + "%  ";
            displayText[4] = String(inputData[6]) + "W  ";

            // GPU Data
            int gpuFanSpeed = (inputData[8] << 8) | inputData[7];
            displayText[5] = String(gpuFanSpeed) + " RPM  ";
            displayText[6] = String(inputData[9]) + "C  ";
            displayText[7] = String(inputData[10]) + "C  ";
            displayText[8] = String(inputData[11]) + "%  ";

            // System data
            if (inputData[12] < 10)
            {
              displayText[9] = "0" + String(inputData[12]) + ":";
            }
            else
            {
              displayText[9] = String(inputData[12]) + ":";
            }

            if (inputData[13] < 10)
            {
              displayText[10] = "0" + String(inputData[13]) + ":";
            }
            else
            {
              displayText[10] = String(inputData[13]) + ":";
            }

            if (inputData[14] < 10)
            {
              displayText[11] = "0" + String(inputData[14]);
            }
            else
            {
              displayText[11] = String(inputData[14]);
            }

            int sysFan1 = (inputData[18] << 8) | inputData[17];
            int sysFan2 = (inputData[20] << 8) | inputData[19];
            int sysFan3 = (inputData[22] << 8) | inputData[21];
            int sysFan4 = (inputData[24] << 8) | inputData[23];

            displayText[14] = String(sysFan1) + " RPM ";
            displayText[15] = String(sysFan2) + " RPM ";
            displayText[16] = String(sysFan3) + " RPM ";
            displayText[17] = String(sysFan4) + " RPM ";

          }
          break;
      }
    }
    else
    {
      fanCount = inputData[15];
      AMDGPU = inputData[16];
      drawDisplay();
    }
    //Clear Message bytes
    for (int i = 0; i < 25; i++)
    {
      inputData[i] = 0;
    }
  }

  //Let the PC know we are ready for more data
  Serial.print("-READY");

  if (timeElapsed > 1000 && displayReady)
  {
    // CPU Data
    TFTscreen.setCursor(32, 39);
    TFTscreen.println(displayText[0]);

    TFTscreen.setCursor(42, 69);
    TFTscreen.println(displayText[1]);

    TFTscreen.setCursor(42, 100);
    TFTscreen.println(displayText[2]);

    TFTscreen.setCursor(42, 130);
    TFTscreen.println(displayText[3]);

    TFTscreen.setCursor(42, 160);
    TFTscreen.println(displayText[4]);

    // GPU Data
    TFTscreen.setCursor(142, 39);
    TFTscreen.println(displayText[5]);

    TFTscreen.setCursor(152, 69);
    TFTscreen.println(displayText[6]);

    TFTscreen.setCursor(152, 100);
    TFTscreen.println(displayText[7]);

    TFTscreen.setCursor(152, 130);
    TFTscreen.println(displayText[8]);

    // System data
    TFTscreen.setCursor(244, 39);
    TFTscreen.print(displayText[9]);
    TFTscreen.print(displayText[10]);
    TFTscreen.print(displayText[11]);

    TFTscreen.setCursor(260, 69);
    TFTscreen.print(displayText[12]);

    TFTscreen.setCursor(260, 100);
    TFTscreen.print(displayText[13]);

    if (fanCount > 0)
    {
      TFTscreen.setCursor(244, 130);
      TFTscreen.print(displayText[14]);
    }

    if (fanCount > 1)
    {
      TFTscreen.setCursor(244, 160);
      TFTscreen.print(displayText[15]);
    }

    if (fanCount > 2)
    {
      TFTscreen.setCursor(244, 190);
      TFTscreen.print(displayText[16]);
    }

    if (fanCount > 3)
    {
      TFTscreen.setCursor(244, 220);
      TFTscreen.print(displayText[17]);
    }
    timeElapsed = 0;
  }

  if (DHTtime > 5000)
  {
    int temp = dht.readTemperature();
    int humidity = dht.readHumidity();

    if (temp < 10)
    {
      displayText[12] = "0" + String(temp) + "C  ";
    }
    else
    {
      displayText[12] = String(temp) + "C  ";
    }

    if (humidity < 10)
    {
      displayText[13] = "0" + String(humidity) + "C  ";
    }
    else
    {
      displayText[13] = String(humidity) + "%  ";
    }
  }
}

void drawDisplay()
{
  int GPUCOL;
  int SYSCOL;
  if (AMDGPU == 1)
  {
    GPUCOL = AMDRED;
    SYSCOL = GREEN;
  }
  else
  {
    GPUCOL = GREEN;
    SYSCOL = AMDRED;
  }
  drawScene();
  TFTscreen.setFont(ArialBlack_13);

  // CPU
  TFTscreen.fillRect(6, 6, 94, 226, ILI9341_BLACK);
  TFTscreen.fillRect(6, 6, 94, 17, LTBLUE);
  TFTscreen.drawRect(6, 6, 94, 226, LTBLUE);
  TFTscreen.setTextColor(ILI9341_BLACK, LTBLUE);
  TFTscreen.setCursor(32, 8);
  TFTscreen.print("CPU");

  // GPU
  TFTscreen.fillRect(113, 6, 94, 226, ILI9341_BLACK);
  TFTscreen.setCursor(140, 14);
  TFTscreen.fillRect(113, 6, 94, 17, GPUCOL);
  TFTscreen.drawRect(113, 6, 94, 226, GPUCOL);
  TFTscreen.setTextColor(ILI9341_BLACK, GPUCOL);
  TFTscreen.setCursor(139, 8);
  TFTscreen.print("GPU");

  // SYSTEM
  TFTscreen.fillRect(220, 6, 94, 226, ILI9341_BLACK);
  TFTscreen.setCursor(231, 14);
  TFTscreen.fillRect(220, 6, 94, 17, SYSCOL);
  TFTscreen.drawRect(220, 6, 94, 226, SYSCOL);
  TFTscreen.setTextColor(ILI9341_BLACK, SYSCOL);
  TFTscreen.setCursor(226, 8);
  TFTscreen.print("SYSTEM");

  TFTscreen.setFont(Michroma_8);

  // CPU Data headings
  TFTscreen.setTextColor(LTBLUE, ILI9341_BLACK);
  TFTscreen.setCursor(38, 26);
  TFTscreen.print("Fan");

  TFTscreen.setCursor(30, 56);
  TFTscreen.print("Temp");

  TFTscreen.setCursor(11, 86);
  TFTscreen.print("Temp (max)");

  TFTscreen.setCursor(32, 116);
  TFTscreen.print("Load");

  TFTscreen.setCursor(27, 146);
  TFTscreen.print("Power");

  // GPU Data headings
  TFTscreen.setTextColor(GPUCOL, ILI9341_BLACK);
  TFTscreen.setCursor(147, 26);
  TFTscreen.print("Fan");

  TFTscreen.setCursor(137, 56);
  TFTscreen.print("Temp");

  TFTscreen.setCursor(118, 86);
  TFTscreen.print("Temp (max)");

  TFTscreen.setCursor(141, 116);
  TFTscreen.print("Load");

  // System Data headings
  TFTscreen.setTextColor(SYSCOL, ILI9341_BLACK);
  TFTscreen.setCursor(248, 26);
  TFTscreen.print("Time");

  TFTscreen.setCursor(238, 56);
  TFTscreen.print("Ambient");

  TFTscreen.setCursor(235, 86);
  TFTscreen.print("Humidity");

  if (fanCount > 0)
  {
    TFTscreen.setCursor(244, 116);
    TFTscreen.print("Fan #1");
  }

  if (fanCount > 1)
  {
    TFTscreen.setCursor(244, 146);
    TFTscreen.print("Fan #2");
  }

  if (fanCount > 2)
  {
    TFTscreen.setCursor(244, 176);
    TFTscreen.print("Fan #3");
  }

  if (fanCount > 3)
  {
    TFTscreen.setCursor(244, 206);
    TFTscreen.print("Fan #4");
  }

  TFTscreen.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  TFTscreen.setTextSize(1);
  TFTscreen.setFontAdafruit();
  
  displayReady = true;
}

void drawScene()
{
   TFTscreen.fillRectVGradient(0, 0, 320, 120, BLACK, PURPLE);
  TFTscreen.fillRectVGradient(0, 120, 320, 120, PURPLE, BLACK);
  
  for (int i = 0; i < 45; i++)
  {
    long x = random(0, 320);
    long y = random(0, 110);
    TFTscreen.drawPixel(x, y, WHITE);
  }

  TFTscreen.drawFastHLine(0, 120, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 125, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 130, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 140, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 150, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 165, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 180, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 200, 320, MAGENTA);
  TFTscreen.drawFastHLine(0, 220, 320, MAGENTA);
  TFTscreen.drawLine(40, 120, 0, 130, MAGENTA);
  TFTscreen.drawLine(60, 120, 0, 140, MAGENTA);
  TFTscreen.drawLine(80, 120, 0, 165, MAGENTA);
  TFTscreen.drawLine(100, 120, 0, 220, MAGENTA);
  TFTscreen.drawLine(120, 120, 0, 320, MAGENTA);
  TFTscreen.drawLine(140, 120, 90, 320, MAGENTA);
  TFTscreen.drawLine(160, 120, 160, 320, MAGENTA);
  TFTscreen.drawLine(180, 120, 230, 320, MAGENTA);
  TFTscreen.drawLine(200, 120, 320, 320, MAGENTA);
  TFTscreen.drawLine(220, 120, 320, 220, MAGENTA);
  TFTscreen.drawLine(240, 120, 320, 165, MAGENTA);
  TFTscreen.drawLine(260, 120, 320, 140, MAGENTA);
  TFTscreen.drawLine(280, 120, 320, 130, MAGENTA);
}

void drawSplash()
{
  drawScene();
  TFTscreen.setFont(ArialBlack_20);
  TFTscreen.setTextColor(BLACK);  
  TFTscreen.setCursor(41, 86);
  TFTscreen.print("H A R D W A R E");
  TFTscreen.setCursor(64, 126);
  TFTscreen.print("M O N I T O R");
  TFTscreen.setCursor(37, 88);
  TFTscreen.setTextColor(WHITE);
  TFTscreen.print("H A R D W A R E");
  TFTscreen.setCursor(60, 128);
  TFTscreen.print("M O N I T O R");
}
