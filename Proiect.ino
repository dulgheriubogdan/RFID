#include <FS.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>


int buzzer = D1;
char ssid[100];
char pass[100];
char nume[100];
String numeStr;

constexpr uint8_t RST_PIN = D0;     
constexpr uint8_t SS_PIN = D8;  
   
MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;
String tag;

void charToString(char S[], String &D)
{
    String rc(S);
    D = rc;
}

void readContent(String &string)
{
    int numarCar=0;
    char car[100];
    File citeste = SPIFFS.open("/config.txt", "r");
   
    if (!citeste) {
      Serial.println("Eroare deschidere fisier citire!");
      return;
    }
  
    Serial.println("Continut fisier:");
    
    while (citeste.available()) {
      
      car[numarCar]=citeste.read();
      numarCar++;
      Serial.write(car[numarCar]);
    }
  
    charToString(car,string);
    
    citeste.close();
}

void clearContent()
{
    File file = SPIFFS.open("/config.txt", "w");
   
    if (!file) {
        Serial.println("Eroare deschidere fisier scriere!");
        return;
    }
     
    int bytesWritten = file.print("");
    Serial.println("Fisier golit");
    
    file.close();
}

void writeOSContent(String string)
{
    File file = SPIFFS.open("/config.txt", "w");
   
    if (!file) {
        Serial.println("Eroare deschidere fisier scriere!");
        return;
    }
     
    int bytesWritten = file.print("DATA|"+string+"|DATA");
    Serial.print("Scrie in fisier: DATA|"+string+"|DATA");
    Serial.println();
    
    if (bytesWritten == 0) {
        Serial.println("Eroare scriere in fisier!");
        return;
    }
    
    file.close();
}

int numeLen=0;

bool identifyParams(char content[])
{
    int loc=0;
    bool ok=false;

    while(loc<500&&ok==false)
    {
      if(content[loc]=='D'&&content[loc+1]=='A'&&content[loc+2]=='T'&&content[loc+3]=='A'&&content[loc+4]=='|')
      {
          loc+=5;
          int poz=0;
          while(content[loc]!=92)
            ssid[poz++]=content[loc++];
          ssid[poz]=NULL;
          loc++;poz=0;
          while(content[loc]!=92)
            pass[poz++]=content[loc++];
          pass[poz]=NULL;
          loc++;numeLen=0;
          while(content[loc]!='|'||content[loc+1]!='D'||content[loc+2]!='A'||content[loc+3]!='T'||content[loc+4]!='A')
            nume[numeLen++]=content[loc++];
          for(int i=0;i<=numeLen;i++)
          {
            Serial.println();
            Serial.print(i);
            Serial.write(nume[i]);
            Serial.println();
            if(!(nume[i]>='0'&&nume[i]<='9')&&!(nume[i]>='a'&&nume[i]<='z')&&!(nume[i]>='A'&&nume[i]<='Z'))
            {
              Serial.print("sters");
              Serial.println();
              i--;
              numeLen--;
            }
          }
          numeLen++;
          nume[numeLen++]='/';
          nume[numeLen]=NULL;
          ok=true;
            
      }
      loc++;
    }
    if(ok==true)
    {
      String content;
      Serial.println("Identificarea parametrilor a fost realizata cu succes!");
      charToString(ssid,content);
      Serial.println(content);
      charToString(pass,content);
      Serial.println(content);
      charToString(nume,content);
      Serial.println(content);
      numeStr=content;
    }
    else
    {
      Serial.println("Identificarea parametrilor NU s-a realizat!");
    }
    return ok;
}

void connectWifi()
{
    Serial.print("Se coneteaza la reteaua ");
    Serial.println(ssid);
    
    WiFi.begin(ssid,pass);
    
    while( WiFi.status() != WL_CONNECTED )
    {
      delay(500);
      Serial.print(".");        
    }
  
    Serial.println();
  
    Serial.println("Wifi conectat cu succes!");
    Serial.print("Adresa nod : ");
    Serial.println(WiFi.localIP() );
}

void setup() {

  pinMode(buzzer, OUTPUT);
  
  Serial.begin(115200);
  
  bool success = SPIFFS.begin();
    
  if (!success) {
    Serial.println("Eroare identificare fisier date!");
    return;
  }

  String content;
  char charCont[500];
  readContent(content);
  content.toCharArray(charCont,500);
  bool res = identifyParams(charCont);
  
  if(res==true)
      connectWifi();
  else
  {
      tone(buzzer,2000,400);
      delay(600);
      tone(buzzer,2000,400);
      delay(600);
      tone(buzzer,2000,400);
      delay(600);
      tone(buzzer,2000,400);
  }
  
  SPI.begin(); 
  rfid.PCD_Init(); 
  
  tone(buzzer,2000,100);
  delay(200);
  tone(buzzer,2000,100);
  delay(200);
  tone(buzzer,2000,100);
  delay(200);
}

String apiKey = "8b53d648-31d6-46f8-93b3-ba65f96d809b";
//const char* fingerprint = "FF F4 1D 3E 8C 2E 66 B4 CA 71 ED 22 52 5D AC BB 8B 7F F8 D4";

int incomingByte=-1;

void loop() {
  int ok=0;
  String content;
  if(incomingByte!=-1)
  {
    if (Serial.available() > 0) {
  
      incomingByte = Serial.read();
      
      if(char(incomingByte)=='r')
      { 
          readContent(content);
          Serial.println(content);
      }
      else
      if(char(incomingByte)=='w')
      { 
          content = Serial.readString();
          writeOSContent(content);
      }
      else
      if(char(incomingByte)=='d')
      {
          clearContent();
          Serial.println("A functionat!");
      }
      else
      if(char(incomingByte)=='c')
      {
          connectWifi();
      }
      else
      if(char(incomingByte)=='i')
      {
        char charCont[500];
        readContent(content);
        content.toCharArray(charCont,500);
        identifyParams(charCont);
      }
      else
      if(char(incomingByte)=='q')
      {
        incomingByte=-1;
      }
      else
      {
         Serial.println(char(incomingByte));
      }
    }
  }
  else
  {
  if(!rfid.PICC_IsNewCardPresent())
    return;
  if(rfid.PICC_ReadCardSerial()){
    for (byte i = 0; i < 4; i++){
      tag += rfid.uid.uidByte[i];
    }
    if( WiFi.status() == WL_CONNECTED )
    {
      tone(buzzer,5000,250);
      delay(500);
      
      HTTPClient http;
      WiFiClientSecure client;
      
      //client.setFingerprint(fingerprint);
      client.setInsecure();

      String url;
      char urlChar[300];
      int loc=0;
      
      String url1="https://web.med-soft.ro/dev/api/integrations/generic/public/dev/rfid_aparate/";
      while(loc < url1.length()) 
      {
         urlChar[loc] = url1.charAt(loc);loc++;
      }
      for(int i=0;i<numeLen;i++)
      {
        urlChar[loc]=nume[i];loc++;
      }
      for(int i=0; i < tag.length(); i++) 
      {
         urlChar[loc] = tag.charAt(i);loc++;
      }
      urlChar[loc]=NULL;
      charToString(urlChar,url);
      Serial.println(url);

      http.begin(client, url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("x-api-key",apiKey);
        
      int httpResponseCode = http.GET();
      String load = http.getString();

      Serial.println(httpResponseCode);
      Serial.println(load);
      if(httpResponseCode==200)
      {
        tone(buzzer,5000,250);
        delay(500);
      }
      http.end();
      
    }
    else
    {
      tone(buzzer,2000,100);
      delay(200);
      tone(buzzer,2000,100);
      delay(600);
      tone(buzzer,2000,400);
      delay(600);
      tone(buzzer,2000,400);
      Serial.println("Eroare la conexiune!");
    }
    
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  }
}
  
