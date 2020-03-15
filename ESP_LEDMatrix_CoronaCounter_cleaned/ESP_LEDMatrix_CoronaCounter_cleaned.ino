/*
  Live Coronavirus (COVID-19) victims counter
  Arduino/ESP8266 + LED matrix display
  (c)2020 Pawel A. Hernik
  YT video:
  https://youtu.be/4z0reqAF2xQ
 
  ESP-01 pinout:
  GPIO 2 - DataIn
  GPIO 1 - LOAD/CS
  GPIO 0 - CLK

  ------------------------
  NodeMCU 1.0 pinout:
  D8 - DataIn
  D7 - LOAD/CS
  D6 - CLK
*/


#include "Arduino.h"
#include <ESP8266WiFi.h>

#define NUM_MAX 4
#define ROTATE 90

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0
#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6

#include "max7219.h"
#include "fonts.h"

// =======================================================================
// Your config below!
// =======================================================================
const char *ssid     = "";      // SSID of local network
const char *password = "";      // Password
// =======================================================================
// URL good for all EU countries
const char *countryURL = "/maps/d/viewer?mid=1yCPR-ukAgE55sROnmBUFmtLN6riVLTu3&ll=54.81124088123701%2C28.707117003395524&z=4";

//const char *countryURL = "/maps/d/viewer?mid=1yCPR-ukAgE55sROnmBUFmtLN6riVLTu3&ll=52.52718300000002%2C19.28310239999996&z=18";
const char *countryString = "Poland - Nationwide";
const int numDescLines = 12;

//const char *countryURL = "/maps/d/viewer?mid=1yCPR-ukAgE55sROnmBUFmtLN6riVLTu3&ll=42.33487101680273%2C12.89892757812504&z=7";
//const char *countryString = "Italy - Nationwide";
//const int numDescLines = 4;

//const char *countryString = "Germany - Nationwide";
//const int numDescLines = 3;

//const char *countryString = "Bulgaria - Nationwide";
//const int numDescLines = 2;

//const char *countryURL = "/maps/d/viewer?mid=1yCPR-ukAgE55sROnmBUFmtLN6riVLTu3&ll=52.017133422046584%2C21.74508962137918&z=7";
//const char *countryString = "POL - Mazowieckie - Warsaw";
//const int numDescLines = 1;

// =======================================================================

/*
Map and data taken from:

https://www.google.com/maps/d/viewer?mid=1yCPR-ukAgE55sROnmBUFmtLN6riVLTu3&ll=52.52718300000002%2C19.28310239999996&z=18

***Map updated every few minutes thanks to https://reddit.com/r/covidmapping team of volunteers***

HTML output:
Poland - Nationwide\\n\"]\n,1]\n,[\"description\",[\"MAR-15-2020 (4:52PM UTC)\\n\\n111 confirmed cases\\n95 active\\n4 critical\\n13 recovered\\n3 dead\\n\\n20340 under surveillance\\n4414 tested\\n4413 quarantined\\n526 hospitalized\\n\\nMAR-13-2020:\\n- Entering Poland by foreigners forbidden\\n- Two-week home quarantine for poles entering Poland\\n- International air and rail connections suspended\\n- Malls activities limited (pharmacies, drugstores and grocery stores are open)\\n- Gastronomy activities limited (restaurants, bars and cafes will be able to provide their services only in the form of \\\"takeaway\\\" and \\\"for delivery\\\")\\n- Gatherings above 50 people forbidden (including religious gatherings)\\n\\nMAR-11-2020:\\n- Suspension of all classes at universities, kindergartens, schools and other educational institutions from March 12 to 25\\n- Cultural institutions, concert halls, operas, theaters, museums and cinemas will be closed from March 12\\n- All mass events have been canceled\\n- Five sanitary control points were created at border crossing points:\\n✔ Kolbaskowo\\n✔ Swiecko\\n✔ Jedrzychowice \\n✔ Olszyna \\n✔ Gorzyczki\\n\\n\\ntests- https://twitter.com/MZ_GOV_PL/status/1238381192199786497\\nhttps://twitter.com/MZ_GOV_PL/status/1238139031860531206\\nhttps://twitter.com/MZ_GOV_PL/status/1238870731972370437/photo/1\\nrecovered: https://www.se.pl/wiadomosci/polska/koronawirus-w-polsce-zaskakujace-dane-gis-13-osob-wyzdrowialo-aa-r9Se-zRiW-ouxT.html\\n13 recovered - MAR-14-2020\\nhttps://twitter.com/PremierRP/status/1238886453226110977\\n\"]\n,1]\n,[]\n,[]\n]\n,null,110]\n,[\"2EE100A23AAA9DB1\",[[[39.5083585,-7.9930713]\n]\n]\n,[]\n,[]\n,0,[[\"name\",[\"Portugal - Nationwide\"]\n,1]\n,[\"description\",[\"169 confirmed cases nationwide (167 active)- 14-MAR

interesting part only:
[\"description\",[\"MAR-14-2020 (4:52PM UTC)\\n\\n104 confirmed cases\\n88 active\\n4 critical\\n13 recovered\\n3 dead\\n\\n20340 under surveillance\\n4414 tested\\n4413 quarantined\\n526 hospitalized\\n

*/
// =======================================================================

void setup() 
{
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,0);
  Serial.print("\nConnecting to WiFi ");
  scrollString(" WiFi ...",15);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(500);
  }
  Serial.println("");
  Serial.print("Connected, my IP: "); Serial.println(WiFi.localIP());
}
// =======================================================================

#define BUF_LEN 2000
char buf[BUF_LEN+1];
char *country, *description, *nl;
int cnt;

void loop()
{
  if(!cnt--) 
  {
    cnt = 50;  // data is refreshed every 50 loops
    Serial.println("Getting data ...");
    scrollString(" Data ...",15);
    if(getCoronaVictims()!=0) { scrollString(" error ",20); cnt=1; }
    //delay(1000);
  }
  if(description) {
    scrollString("    ",20);
    scrollString(countryString,20);
    scrollString("   ",20);
    scrollString(description,40);
  }
}
// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

// =======================================================================

void scrollChar(unsigned char c, int scrollDelay) 
{
  if(c<' ' || c>MAX_CHAR) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(scrollDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void scrollString(const char* s, int shiftDelay)
{
  while(*s) scrollChar(*s++, shiftDelay);
}

// =======================================================================

const char* host = "www.google.com";
const int httpsPort = 443;
const int httpPort = 80;
int ll = 0;

int getCoronaVictims()
{
  description = NULL;
  WiFiClientSecure client;
  Serial.print("Connecting to "); Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed!");
    return -1;
  }
  client.print(String("GET ") + countryURL + " HTTP/1.1"
    + "\r\nHost: " + host 
    //+ "\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:54.0) Gecko/20100101 Firefox/74.0"
    + "\r\nConnection: close\r\n\r\n");

  int repeatCounter = 50;
  while(!client.available() && repeatCounter--) { Serial.println("c."); delay(500); }
  Serial.println("Connected.");
  while(client.connected() && client.available()) {
    client.readBytes(buf,BUF_LEN);
    buf[BUF_LEN]=0;
    //Serial.println(buf);
    country = strstr(buf, countryString);
    if(country) {
      Serial.println(country); Serial.print("country at: "); ll = country-buf; Serial.println(ll);
      description = strstr(country+strlen(countryString),"description");
      if(description && description<country+strlen(countryString)+20) {
        Serial.print("description at: "); ll = description-buf; Serial.println(ll);
        description += strlen("description")+6;
        nl = description;
        //Serial.println(description);
        for(int i=0;i<numDescLines;i++) {
          if(nl) nl = strstr(nl,"\\\\n");
          if(nl) { *nl++=' '; *nl++=' '; *nl++=' '; }
        }
        if(nl) *nl=0;
        Serial.println(description);
        break;
     }
    }
  }
  client.stop();
  return 0;
}
