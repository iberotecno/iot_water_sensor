#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SimpleTimer.h>

// data to configure by user
#define CHAT_ID "Your chat ID"
#define BOTtoken "Your Bot Token"  // your Bot Token (Get from Botfather)
char ssid[] = "Your Wifi name";     // your network SSID (name)
char password[] = "Your Wifi Pass"; // your network key
int horas = 1; // horas cada cuanto reiniciamos por seguridad

// variables definitions and initializations
int flag = 0;
int acu = 0;
unsigned long previousMillis = 0;
const long interval = 1500;
SimpleTimer timer;
const int waterSensor = 33;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

// restart function definition
void reinicio() {
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  timer.setInterval(horas * 3600000, reinicio); // call to restart function if configured time is reached. It improves stability.
  
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
    WiFi.mode(WIFI_OFF); //wifi goes off to save power
  }
}
