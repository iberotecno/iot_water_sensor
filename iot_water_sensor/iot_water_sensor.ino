// iberotecno 2020 

#include <FS.h>                       //library to access the filesystem
#include <WiFiClientSecure.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>       //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ESPAsyncWiFiManager.h>     //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h>             //https://github.com/bblanchon/ArduinoJson
#include <UniversalTelegramBot.h>    //https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <SimpleTimer.h>             //https://github.com/jfturcot/SimpleTimer

//define custom fields
char chat_id[100];
char bot_token[100];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// variables definitions and initializations
int flag = 0;
int acu = 0;
const int waterSensor = 33;
AsyncWebServer server(80);
DNSServer dns;
SimpleTimer timer;


void setup() {
Serial.begin(115200);
  Serial.println("\n Starting");
  //
  //***clean FS, for testing
  //SPIFFS.format();
 
  //read configuration from FS json
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(chat_id, json["chat_id"]);
          strcpy(bot_token, json["bot_token"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  //Telegram parameters
  AsyncWiFiManagerParameter custom_chat_id("chat_id", "chat id", chat_id, 100);
  AsyncWiFiManagerParameter custom_bot_token("bot_token", "bot token", bot_token, 100);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server, &dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_chat_id);
  wifiManager.addParameter(&custom_bot_token);

  //scheduled reset
  int horas = 1; // hours for scheduled reset
  timer.setInterval(horas * 3600000, reinicio);

  //WiFiManager

  AsyncWiFiManagerParameter custom_text("<p>Seleccione la red WiFi para conectarse.</p>");
  wifiManager.addParameter(&custom_text);
  //reset saved settings
  //wifiManager.resetSettings();
  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Iberotecno AP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters

  strcpy(chat_id, custom_chat_id.getValue());
  strcpy(bot_token, custom_bot_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["chat_id"] = chat_id;
    json["bot_token"] = bot_token;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}

void loop() {
  timer.run();
  int valor = analogRead(waterSensor);
  if (valor >= 5) { // call alert function if water is detected
    if (flag == 0) {
      alerta();
    }
  } else {
    flag = 0;
  }
  delay(100);
}

void alerta() { //alert function
  flag = 1;
  // Initialize Telegram BOT
  WiFiClientSecure client;
  UniversalTelegramBot bot(bot_token, client);
  int Bot_mtbs = 1000; //mean time between scan messages
  long Bot_lasttime;   //last time messages' scan has been done

  //Send alert
  String message = "Agua detectada";
  if (bot.sendMessage(chat_id, message, "Markdown")) {
    Serial.println("TELEGRAM Successfully sent");
  } else {
    Serial.println("Fallo TELEGRAM");
    reinicio();
  }
}

// restart function definition
void reinicio() {
  ESP.restart();
}
