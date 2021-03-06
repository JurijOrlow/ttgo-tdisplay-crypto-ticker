//Includes-------------------------------------------|

#include "WiFi.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>

//Defines--------------------------------------------|

#define ONBOARD_LED 25

//Constants and Variables----------------------------|

char chPassword[] = "xxx";
char chSSID[] = "xxx";

char chBuffer[128];

float prices[2];

float myBalance[2] = {0.0, 0.0};
float pln = 4;
float wealthEur;

long ntpPeriod = 3600000;

unsigned long ntpTimingStart;
unsigned long ntpTimingStop;
unsigned long startTime;
unsigned long stopTime;

bool flag = 0;
bool cryptoChose = 0;
bool goToSleep = 0;

String formattedDate;
String dayStamp;
String timeStamp;

String cryptos[2] = {"ETH", "BTC"};

String currencies[2] = {"EUR", "PLN"};

unsigned long exTime;
unsigned long toExTime;

//Setup----------------------------------------------|

void IRAM_ATTR cryptoSwitch()
{
  cryptoChose = !cryptoChose;
  Serial.println(cryptoChose);
}

void IRAM_ATTR deepSleep()
{
  goToSleep = 1;
}

TFT_eSPI tft = TFT_eSPI();

StaticJsonDocument<4096> jsonBuffer;

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP);

void setup() {

  Serial.begin (115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  pinMode(ONBOARD_LED, OUTPUT);

//WiFi Connection------------------------------------|
  Serial.print("Connecting to Wifi");
  WiFi.begin (chSSID, chPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay (1250);
  }

  char  chIp[81];
  WiFi.localIP().toString().toCharArray(chIp, sizeof(chIp) - 1);
  sprintf(chBuffer, "IP  : %s", chIp);
  Serial.println(chBuffer);
  tft.drawString(chBuffer, 0, 0, 2);

  sprintf(chBuffer, "RSSI: %d", WiFi.RSSI());
  Serial.println(chBuffer);
  tft.drawString(chBuffer, 0, 20, 2);
  delay (100);

  timeClient.begin();
  timeClient.setTimeOffset(3600);

  pinMode(0, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(0), cryptoSwitch, FALLING);
  attachInterrupt(digitalPinToInterrupt(35), deepSleep, FALLING);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);

  startTime = millis();
  ntpTimingStart = millis();
}


void loop() {

  stopTime = millis();
  if(stopTime - startTime >= 5000)
  {
    unsigned long execTime = millis();
    if(goToSleep == 1)
    {
      detachInterrupt(digitalPinToInterrupt(32)); //because later used for wake up
      //Serial.println("Going to sleep now");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35,0); //1 = High, 0 = Low connected to GPIO32
      esp_deep_sleep_start();
    }

    if ((WiFi.status() == WL_CONNECTED))
    {
      while(!timeClient.update())
      {
        timeClient.forceUpdate();
      }
      HTTPClient http;

      String priceString = "https://api.binance.com/api/v1/ticker/price?symbol=" + cryptos[cryptoChose] + currencies[0];
      http.begin(priceString);
      http.GET();
      String payload = http.getString();
      deserializeJson(jsonBuffer, payload);
      prices[0] = jsonBuffer["price"];
      http.end();

      String changeString = "https://api.binance.com/api/v3/ticker/24hr?symbol=" + cryptos[cryptoChose] + currencies[0];
      http.begin(changeString);
      http.GET();
      payload = http.getString();
      deserializeJson(jsonBuffer, payload);
      prices[1] = jsonBuffer["priceChangePercent"];
      http.end();

      ntpTimingStop = millis();
      if(ntpTimingStop - ntpTimingStop >= ntpPeriod || flag == 0)
      {
        String currencyString = "https://open.er-api.com/v6/latest/" + currencies[0];
        http.begin(currencyString);
        int code = http.GET();
        payload = http.getString();
        deserializeJson(jsonBuffer, payload);
        JsonObject rates = jsonBuffer["rates"];
        pln = rates[currencies[1]];
        http.end();
        startTime = millis();
        flag = 1;
      }

      wealthEur = prices[0] * myBalance[cryptoChose];
      float wealthPln = wealthEur * pln;

      formattedDate = timeClient.getFormattedDate();
      int splitT = formattedDate.indexOf("T");
      dayStamp = formattedDate.substring(0, splitT);
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

      String cryptoPrice1 = cryptos[cryptoChose] + " price (" + currencies[0] + "): " + String(prices[0]);
      String cryptoPrice2 = cryptos[cryptoChose] + " price (" + currencies[1] + "): " + String(prices[0] * pln);
      String cryptoChange1 = cryptos[cryptoChose] + " 24h change: ";
      String cryptoChange2 = String(prices[1]) + "%";
      String realBalance = "Balance: " + String(wealthEur) + " " + currencies[0] + "/" + String(wealthPln) + " " + currencies[1];

      tft.fillScreen(TFT_BLACK);
      tft.drawString(dayStamp, 0, 0, 2);
      tft.drawString(timeStamp, 0, 20, 2);
      tft.drawString(cryptoPrice1, 0, 40, 2);
      tft.drawString(cryptoPrice2, 0, 60, 2);
      tft.drawString(cryptoChange1, 0, 80, 2);
      if(prices[1] >= 0)
      {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
      }
      else if(prices [1] < 0)
      {
        tft.setTextColor(TFT_RED, TFT_BLACK);
      }
      tft.drawString(cryptoChange2, 110, 80, 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(realBalance, 0, 100, 2);
      String timing = "Fetch time: " + String(millis() - stopTime) + " ms";
      tft.drawString(timing, 0, 120, 2);
    }
    startTime = millis();
  }
}
