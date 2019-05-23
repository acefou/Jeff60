
/*
 *  acefou 12 mai 2019
  */

 
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <WEMOS_SHT3X.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "JEFF 60 ans"
#define APPSK  ""
#endif
const char *ssid = APSSID;
const char *password = APPSK;


SHT3X TH1sht30(0x45);
float TH1tempC;
float TH1humidite;

float TH2tempC;
float TH2humidite;

int ret;

#define TFT_DC     D3 //8
#define TFT_CS     D4 //10
#define TFT_RST    9  // 
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

int px1, py1, pw1, ph1, px2, py2, pw2, ph2;
int px3, py3, pw3, ph3;
int hpol2 = 13; // hauteur police size 2;

const char* version_croquis = "acefou v2019 05 12 Jeff 60";
String nom_sketch = "Jeff 60";

String tInt = "Th: ";
//String tExt = "Th2:";
//String tTH3 = "Th3:";



ESP8266WebServer  server(80);

int status = WL_IDLE_STATUS;

char ID_monESP[ 16 ];
String Temp1;
String Temp2;

unsigned long TopRafraichissementTH1;
unsigned long TopRafraichissementTH2;
unsigned long TopRafraichissementTH3;

unsigned long DelaiNettoyageZoneTH1 = 3000; // une mesure de + de 3 secondes n'est plus pertinente et doit être effacée
unsigned long DelaiNettoyageZoneTH2 = 10000; // une mesure de + de 10 sec n'est plus pertinente et doit être effacée
unsigned long DelaiNettoyageZoneTH3 = 10000; // une mesure de + de 10sec  n'est plus pertinente et doit être effacée

boolean bVideTH2;
boolean bVideTH3;
boolean bOccupe;

//RemoteDebug Debug;
String sHID;

void logMoiCa(String Msg, bool NL) {
  Serial.print(Msg);
  //Debug.print(Msg);
  if (NL==true){
    Serial.println("");
    //Debug.println("");
  }
} 

void handleRoot() {
  String Lapage;

  Lapage = "<!DOCTYPE html><html><head><title>60 ans JEFF</title>";
  Lapage = Lapage + "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />";
  Lapage = Lapage + "<meta http-equiv='refresh'      content='30'>";
  Lapage = Lapage + "<style> .button {  background-color: #4CAF50;   border: none;   color: white; font-size: 48px; border-radius: 12px;width: 100%; padding: 15px 25px;    text-align: center;  cursor: pointer; }";
  Lapage = Lapage + ".button:hover {  background-color: green;}</style>";
  Lapage = Lapage + "</head><body>";
  Lapage = Lapage + "<h1 style='color:red;font-size:400%;text-align:center;'>Bonjour JEFF !</h1><BR>";
  Lapage = Lapage + "<h1 style='color:blue;font-size:300%;text-align:center;'>60 ans, cela se fête !</h1><BR>";
  Lapage = Lapage + "<h3 style='font-size:200%;text-align:center;'>Où je suis, la température actuelle est de: ";
  Lapage = Lapage + (String)TH1tempC + "°</h3>";
  Lapage = Lapage + "<h3 style='color:red;'><em>" + Temp2 + "</em></h3><BR><BR>" ;
  Lapage = Lapage + "<p style='color:blue;font-size:400%;text-align:center;'><em>Alors, devinette : Qu'est-ce que c'est ce truc  ?</em></p><BR><BR><BR><BR><BR>";
  Lapage = Lapage + "<p style='color:green;font-size:200%;text-align:center;'><a href='saperpipolette'>Réponse</a></p>";

  Lapage = Lapage + "</body></html>";
  server.send(200, "text/html", Lapage);
}


void handlesaperpipolete(){
  String Lapage;

  Lapage = "<!DOCTYPE html><html><head><title>60 ans JEFF</title>";
  Lapage = Lapage + "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />";
  Lapage = Lapage + "<meta http-equiv='refresh'      content='30'>";
  Lapage = Lapage + "</head><body>";
  Lapage = Lapage + "<h1 style='color:black;font-size:300%;text-align:center;'>Petit cadeau : Le plus petit microcontroleur serveur Web et point d'accès Wifi que je connaisse !</h1><BR>";
  Lapage = Lapage + "<h1 style='color:black;font-size:200%;text-align:center;'>Super rigolo à programmer !</h1><BR>";
  Lapage = Lapage + "<BR><BR><h3>je répète, " +Temp2 + " ! </h3><BR><BR>" ;
  Lapage = Lapage + "<p><a href='javascript:history.go(-1)'>Retour</a></p>";
  Lapage = Lapage + "</body></html>";
 
  server.send(200, "text/html", Lapage);
}


void drawtextTH1(String text, uint16_t color) {
  tft.fillRect(0, py1 - 1, pw1, ph1, ST77XX_WHITE);
  tft.setCursor(px1, py1);
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void drawtextTH2(String text, uint16_t color) {
  tft.fillRect(0, py2 - 1, pw2, ph2, ST77XX_WHITE);
  tft.setCursor(px2, py2);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print(text);
}

void drawtextTH3(String text, uint16_t color) {
  tft.fillRect(0, py3 - 1, pw3, ph3, ST77XX_WHITE);
  tft.setCursor(px3, py3);
  tft.setTextSize(3);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void ViderZoneTH1() {
  tft.fillRect(0, py1 - 1, pw1, ph1, ST77XX_WHITE);
}
void ViderZoneTH2() {
  tft.fillRect(0, py2 - 1, pw2, ph2, ST77XX_WHITE);
  bVideTH2 = true;
}
void ViderZoneTH3() {
  tft.fillRect(0, py3 - 1, pw3, ph3, ST77XX_WHITE);
  bVideTH3 = true;
}


void piedPage(String text, uint16_t color) {
  tft.setCursor(1, 118); //112
  tft.setTextSize(1);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}



void piedPageACEFOU() {

  String pdp;
  pdp = "acefou " + nom_sketch;
  piedPage(pdp, ST77XX_BLACK);
}

void setup() {
  int i;
  TopRafraichissementTH1 = millis(); //init 1ere mesure
  Serial.begin(115200);  //(9600);
  delay(1000);
  Serial.println();
  Serial.println(version_croquis);
  Serial.println(nom_sketch);
  Serial.println("Hello!");

  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST77XX_WHITE);
  tft.setRotation(2); //fils par le haut


  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/saperpipolette",handlesaperpipolete);
  server.begin();
  Serial.println("HTTP server started");
  
  // on commence a ecouter les requetes venant de l'exterieur
  server.begin();
  Serial.println("Server démarré");


  uint32_t n = ESP.getChipId();
  snprintf(ID_monESP, sizeof ID_monESP, "TH1%08X", (unsigned long)n);
  Serial.println(ID_monESP);

  px1 = 4;
  py1 = 12; // 30;
  pw1 = 128;
  ph1 = hpol2 + 2;

  px2 = 4;
  py2 = 30;  // 50; //80 ;
  pw2 = 128;
  ph2 = hpol2 + 2;

  px3 = 4;
  py3 = 45; //85;
  pw3 = 128;
  ph3 = hpol2 + 2;

  piedPageACEFOU();

  bOccupe = false;
  bVideTH2 = false;
  bVideTH3 = false;

  Serial.println("Goto Loop !");
  Serial.println();
}

void loop() {

  server.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis > (DelaiNettoyageZoneTH1 + TopRafraichissementTH1)) {
    
    // il est temps de prendre une nouvelle mesure TH1
    Serial.println("Prise de mesure TH1 : ");
    TopRafraichissementTH1 = millis();
    //1) prise de  mesures TH1
    if (TH1sht30.get() == 0) {
      TH1tempC = TH1sht30.cTemp;
      TH1humidite = TH1sht30.humidity;
      Serial.print("TH1 Temperature in Celsius : ");
      Serial.println(TH1tempC);
      Serial.print("TH1 Humidité relative : ");
      Serial.println(TH1humidite);


      if (TH1tempC > 0) {
        Temp1 = tInt + "+" + TH1tempC;
      } else {
        Temp1 = tInt + "-" + TH1tempC;
      }

      if (TH1tempC<25){
        Temp2="il fait soif";
      }else{
        Temp2="il fait tres soif !";
      }
      Serial.println(Temp2);
      ViderZoneTH2();
      drawtextTH2(Temp2, ST77XX_BLUE);

      
      //2) Mise à jour Affichage
      drawtextTH1(Temp1, ST77XX_GREEN);

      //3) Envoi mesures à Mayotte
      //Serial.println("Envoi mesure TH1 à Mayotte");
      //MiseAjourMayotte(ID_monESP, (String)TH1tempC, (String)TH1humidite);
      Serial.println();

    }
    else
    {
      Serial.println("Error TH1");
      ViderZoneTH1();
         }


  }

  if ((currentMillis > (DelaiNettoyageZoneTH3 + TopRafraichissementTH3)) && !bVideTH3) {
    // il est temps de nettoyer la zone TH2 car il n'y a pas eu de sollicitation sur /TH3 depuis trop longtemps
    ViderZoneTH3();
    Temp1="Bons   60 ans  JEFF";
    drawtextTH3(Temp1, ST77XX_RED);
  }


}



