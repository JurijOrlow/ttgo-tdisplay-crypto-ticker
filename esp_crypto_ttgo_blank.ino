
//Includes-------------------------------------------|

#include "WiFi.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define ONBOARD_LED 25

//Constants and Variables----------------------------|

char chPassword[] = "xxx";
char chSSID[] = "xxx";

char chBuffer[128];

float prices[2];

float myBalance = 0.01193475;
float pln = 4;

long period = 3600000;

unsigned long startTime;
unsigned long currentTime;

bool flag = 0;

String formattedDate;
String dayStamp;
String timeStamp;

unsigned long exTime;
unsigned long toExTime;
//Setup----------------------------------------------|

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

  Serial.print("Connecting to Wifi");
  WiFi.begin (chSSID, chPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay (1250);
  }

  // Display the IP.

  char  chIp[81];
  WiFi.localIP().toString().toCharArray(chIp, sizeof(chIp) - 1);
  sprintf(chBuffer, "IP  : %s", chIp);
  Serial.println(chBuffer);
  tft.drawString(chBuffer, 0, 0, 2);

  // Display the rssi.

  sprintf(chBuffer, "RSSI: %d", WiFi.RSSI());
  Serial.println(chBuffer);
  tft.drawString(chBuffer, 0, 20, 2);
  delay (100);

  timeClient.begin();
  timeClient.setTimeOffset(3600);
  
  startTime = millis();
}


void loop() {
  exTime = millis();
  if ((WiFi.status() == WL_CONNECTED))

  {
    while(!timeClient.update())
    {
      timeClient.forceUpdate();
    }
    HTTPClient http;

    http.begin("https://api.binance.com/api/v1/ticker/price?symbol=ETHEUR");
    http.GET();
    String payload = http.getString();
    deserializeJson(jsonBuffer, payload);
    prices[0] = jsonBuffer["price"];
    http.end();

    http.begin("https://api.binance.com/api/v3/ticker/24hr?symbol=ETHEUR");
    http.GET();
    payload = http.getString();
    deserializeJson(jsonBuffer, payload);
    prices[1] = jsonBuffer["priceChangePercent"];
    http.end();

    currentTime = millis();
    if(currentTime - startTime >= period || flag == 0)
    {
      http.begin("https://open.er-api.com/v6/latest/EUR");
      int code = http.GET();
      //Serial.println(code);
      payload = http.getString();
      //Serial.println(payload);
      deserializeJson(jsonBuffer, payload);
      JsonObject rates = jsonBuffer["rates"];
      pln = rates["PLN"];
      //Serial.println(pln);
      http.end();
      startTime = millis();
      flag = 1;
    }

    float wealthEur = prices[0] * myBalance;
    float wealthPln = wealthEur * pln;

    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

    /*String toPrint = "ETH price: " + String(prices[0]) + " EUR, 24h change: " + String(prices[1]) + "%, my balance: " + String(wealthEur) + " EUR/" + String(wealthPln) + " PLN";
    Serial.println("<----------------------------------------------->\n");
    Serial.println(dayStamp);
    Serial.println(timeStamp);
    Serial.println(toPrint);
    Serial.println("\n<----------------------------------------------->\n");*/

    String ethPrice1 = "ETH price (EUR): " + String(prices[0]);
    String ethPrice2 = "ETH price (PLN): " + String(prices[0] * pln);
    String ethChange1 = "ETH 24h change: ";
    String ethChange2 = String(prices[1]) + "%";
    String myBalance = "Balance: " + String(wealthEur) + " EUR/" + String(wealthPln) + " PLN";
    //tft.setCursor(0,0);
    //tft.print(toPrint);
    tft.fillScreen(TFT_BLACK);
    tft.drawString(dayStamp, 0, 0, 2);
    tft.drawString(timeStamp, 0, 20, 2);
    tft.drawString(ethPrice1, 0, 40, 2);
    tft.drawString(ethPrice2, 0, 60, 2);
    tft.drawString(ethChange1, 0, 80, 2);
    if(prices[1] >= 0)
    {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    }
    else if(prices [1] < 0)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    tft.drawString(ethChange2, 110, 80, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(myBalance, 0, 100, 2);

    toExTime = millis();
    delay(5000-(toExTime - exTime)); //  seconds to update.
  }
}
