#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SimpleTimer.h>
#define CHAT_ID "Your chat ID"
int flag = 0;
int acu = 0;
int horas = 1; // horas cada cuanto reiniciamos por seguridad
unsigned long previousMillis = 0;
const long interval = 1500;
SimpleTimer timer;
const int waterSensor = 33;

// Initialize Wifi connection to the router
char ssid[] = "Your Wifi name";     // your network SSID (name)
char password[] = "Your Wifi Pass"; // your network key

// Initialize Telegram BOT
#define BOTtoken "Your Bot Token"  // your Bot Token (Get from Botfather)
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

void reinicio() {
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  timer.setInterval(horas * 3600000, reinicio);
}
void loop() {
  timer.run();
  int valor = analogRead(waterSensor);
  if (valor >= 5) {
    if (flag == 0) {
      alerta();
    }
  } else {
    flag = 0;
  }
  delay(100);
}

void alerta() {
  flag = 1;
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    acu++;
    delay(500);
    if (acu == 30) {
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  String message = "Agua detectada";
  if (bot.sendMessage(CHAT_ID, message, "Markdown")) {
    Serial.println("TELEGRAM Successfully sent");
    WiFi.mode(WIFI_OFF); //apagar wifi par ahorrar bateria
  }
}
