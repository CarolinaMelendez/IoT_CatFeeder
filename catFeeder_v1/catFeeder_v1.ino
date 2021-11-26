#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Stepper.h>
#include <ArduinoJson.h>
#define A 21
#define B 19
#define C 18
#define D 5
const int ECHO_PIN = 26;
const int TRIGGER_PIN = 27;


const char* WIFI_SSID = "network";
const char* WIFI_PASS = "password";
const char* MQTT_BROKER = "aqg8dgtz1k884-ats.iot.us-east-2.amazonaws.com";
const int MQTT_PORT = 8883;


const int stepsPerRevolution = 2048; 
Stepper myStepper(stepsPerRevolution, A,C,B,D);

const char* CLIENT_ID = "Cliente_ucb_Grupo2_57469"; // unique client id



const char* IN_TOPIC = "ucb/testIoT_catFeeder/Carolina";   
const char* OUT_SHADOW_CAT_FEEDER = "$aws/things/MyThing/shadow/update";
const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";


WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);


StaticJsonDocument<JSON_OBJECT_SIZE(1)> inputDoc;
// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  // Receive message
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }
  if (String(topic) == IN_TOPIC) {
    Serial.println("Message from topic " + String(IN_TOPIC) + ":" + message);

    DeserializationError err = deserializeJson(inputDoc, payload);
    if (!err) {
      String action = String(inputDoc["action"].as<char*>());
      takeActions_onStepper(action);
     }
   }

}

void takeActions_onStepper(String message){
  int steps = 2048;
    if ( message == "FULL"){
      myStepper.step(steps);
      myStepper.step(steps);
    }else if(message == "HALF"){
      myStepper.step(steps);
    }
}
void publishInShadow(){
  int cm = 0.01723 * readUltrasonicDistance(TRIGGER_PIN, ECHO_PIN);
  String messageReturn = "{\"state\":{\"desired\":{\"distance\":"+String(cm)+"}}}";
  mqttClient.publish(OUT_SHADOW_CAT_FEEDER, messageReturn.c_str());
}

void setup() {
  Serial.begin(115200);
  myStepper.setSpeed(10);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while(1) delay(100);
  }
  Serial.println("Connected to " + String(WIFI_SSID));

  wifiClient.setCACert(AMAZON_ROOT_CA1);
  wifiClient.setCertificate(CERTIFICATE);
  wifiClient.setPrivateKey(PRIVATE_KEY);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)){
    Serial.println("Connected to " + String(MQTT_BROKER));
    mqttClient.subscribe(IN_TOPIC);
    mqttClient.subscribe(OUT_SHADOW_CAT_FEEDER);
    Serial.println("Subscribed to " + String(IN_TOPIC));
    Serial.println("Subscribed to topics ");
  } else {
    Serial.println("Couldn't connect to MQTT broker.");
  }
  return mqttClient.connected();
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

void loop(){
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    // Connect to MQTT broker
    if (now - previousConnectMillis >= 5000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) {
        previousConnectMillis = 0;
      } else {
        delay(1000);
        }
    }
  } else {
    // This should be called regularly to allow the client to process incoming 
    // messages and maintain its connection to the server
    publishInShadow();
    mqttClient.loop(); // Here go to function "callback"
    delay(100);
  }
}
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  //-Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  //- Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}
