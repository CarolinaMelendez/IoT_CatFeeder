#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Stepper.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

// pins fron stepper motor
const int A = 21;
const int B = 19;
const int C = 18;
const int D = 5;
const int stepsPerRevolution = 2048; 
Stepper myStepper(stepsPerRevolution, A,C,B,D);

// pins from ultrasonic sensor
const int ECHO_PIN = 26;
const int TRIGGER_PIN = 27;

// AWS broker
const char* MQTT_BROKER = "aqg8dgtz1k884-ats.iot.us-east-2.amazonaws.com";
const int MQTT_PORT = 8883;
const char* CLIENT_ID = "Cliente_ucb_Grupo2_57469"; // unique client id

// Topicos
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
)KEY";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiManager wiFiManager;

StaticJsonDocument<JSON_OBJECT_SIZE(1)> inputDoc;

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  // Receive message
  String message;
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
    if ( message == "FULL"){
      myStepper.step(stepsPerRevolution);
      myStepper.step(stepsPerRevolution);
    }else if(message == "HALF"){
      myStepper.step(stepsPerRevolution);
    }
}

int publishInShadow(){
  int cm = 0.01723 * readUltrasonicDistance(TRIGGER_PIN, ECHO_PIN);
  String messageReturn = "{\"state\":{\"desired\":{\"distance\":"+String(cm)+"}}}";
  mqttClient.publish(OUT_SHADOW_CAT_FEEDER, messageReturn.c_str());
  return cm;
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();

  myStepper.setSpeed(10);
  
  wifiClient.setCACert(AMAZON_ROOT_CA1);
  wifiClient.setCertificate(CERTIFICATE);
  wifiClient.setPrivateKey(PRIVATE_KEY);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

void connectToWiFi(){
  wiFiManager.resetSettings();
  wiFiManager.autoConnect("ESP32_CAT-FEEDER","miau123456789");
  Serial.println("ESP32_CAT-FEEDER is connected to Wi-Fi network");  
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)){
    Serial.println("Connected to " + String(MQTT_BROKER));
    mqttClient.subscribe(IN_TOPIC);
    mqttClient.subscribe(OUT_SHADOW_CAT_FEEDER);
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
  }else {
    sentDistanceAnalytics();
    mqttClient.loop(); // Here go to function "callback"
    delay(100);
  }
}

int cm = 1000;
bool has_published_to_data = false;
unsigned long timeout = millis();

void sentDistanceAnalytics(){
  if ( ! has_published_to_data ){
        cm = publishInShadow();
        delay(500);
      }
      
      if (cm < 35 ){
        Serial.println("distance sent"+ String(cm));
        has_published_to_data = true;
        if (millis() - timeout > 480000) {
          timeout = millis(); 
          has_published_to_data = false;
          }
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
