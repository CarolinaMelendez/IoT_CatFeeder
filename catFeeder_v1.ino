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


const char* WIFI_SSID = "FamT&A";
const char* WIFI_PASS = "40186467";
const char* MQTT_BROKER = "aqg8dgtz1k884-ats.iot.us-east-2.amazonaws.com";
const int MQTT_PORT = 8883;


const int stepsPerRevolution = 2048; 
Stepper myStepper(stepsPerRevolution, A,C,B,D);

const char* CLIENT_ID = "Cliente_ucb_Grupo2_57469"; // unique client id



const char* IN_TOPIC = "ucb/testIoT_catFeeder/Carolina";   
const char* OUT_SHADOW_CAT_FEEDER = "$aws/things/MyThing/shadow/update";
const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUUSEcUa+VvMblWsXDANQOmEcoLi0wDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMTAxOTAyMDg0
M1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMzP/tPlFCGqRO59tnu3
HMtylB5uWbwPuDC7olnVplWXZWUAVsb3yIEG8h1N0dXCgq1fL/MGXwNjdUiCyOS3
f/L//Wu11sSHgQAo2osHJvkTIBKx/eKmYXOePYou2kyOk9Cyj/icDojBxsi8tpWk
PEWgj2ncCed0Nt9hmGb0uaKtTyQ2mrzj8ee0LQHlWktivPirok3qkthuz+YWJXuH
Ote6p9mPMZHpfz1j/qKOHiu1bOhj7iHFCnI5exioYGrmdX/nNRKZwnCj2G2NZylG
be75u4sW+EiufYN/8vfFhqdqoi7Gw6kDgvJijN+ovFGtJqaosOXz/mv7M+43p1j7
uLUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUeTJAboADE/EeZtxlkKdc0IWWubgwHQYD
VR0OBBYEFBRHWyo3fHMr75AXjRkKip7dR0eJMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAR/gwi2D+26pV0hfd5j+lvOr4H
qMur5718PykRx1EVrlx58uF4spVASi+bJKAey33GRc58QjaLPZ0SDWzgk0vw0PVn
zyFMZ+6jDG57vLnIeoqvFYyaMUVNTbsjVlRw7W+becOyPznH5a61fGiAqJ7/B4pk
oS8IbTB781HEPp8abBsuNZFwDrjcUoKfDnmNvPqx4LO9wEK/NtwetBtZPbLBkrHM
xZHdmYrFzX4d7d8LduA7w5DQ6Vryr4Tg1/jyI9o5/fAi/PQApc8SmflJYaGu818m
CeR0TEmrNa95SieGsywAx5fhOe5TYgEhHpC96nIZa+SqHILomYO8ZPRk9L9a
-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAzM/+0+UUIapE7n22e7ccy3KUHm5ZvA+4MLuiWdWmVZdlZQBW
xvfIgQbyHU3R1cKCrV8v8wZfA2N1SILI5Ld/8v/9a7XWxIeBACjaiwcm+RMgErH9
4qZhc549ii7aTI6T0LKP+JwOiMHGyLy2laQ8RaCPadwJ53Q232GYZvS5oq1PJDaa
vOPx57QtAeVaS2K8+KuiTeqS2G7P5hYle4c617qn2Y8xkel/PWP+oo4eK7Vs6GPu
IcUKcjl7GKhgauZ1f+c1EpnCcKPYbY1nKUZt7vm7ixb4SK59g3/y98WGp2qiLsbD
qQOC8mKM36i8Ua0mpqiw5fP+a/sz7jenWPu4tQIDAQABAoIBAQCN2A8YGTLUDeQn
d5EGJU8f536FhM9I22oo9gn+RWUpKezjAlUW/0wxb+oGOXr1m/x8rcu7d9Iduqfj
TBBttlg6puePzSOsgFvrDGAuoQlwvrobuIGokqozNIhSRxiQpL3ib4geGZZHSEyv
gWLCSDgl0IkCO43waiAGnMS7qQlXi4pxNmzO0Tl7chF3AY43A8tp9na5hHOBFh71
7TpmK8vvVTuN2/pVhsPQWH22dZ0puXdj9Cd0zSfMX7/eWf7eKsKvBgmJeszLbKvG
Bzy2kGLVlR59xqAZ6Gu25tjer4aPy7SCHA+xX6NiDOpr+/XmjZ/AsmspuE5Um8x6
TmmhJJ2RAoGBAOczjlHpngrhKkxuCK34NZ8+UzEwoC7A0ByDHzNmMJf7oN6zglZk
yxssCvh5j2MN0BBcJUdp3MSkYK3+gE0ev4sqhVeiF5Cgx5F01aayAWUcfAg2Z7F0
3tv23hx+FFTU34o105DDBaQNQTPJy8w/WCP3weTnG+HvpQ9Q7YKrkVJTAoGBAOLH
1u3lt22H1p/vJkKlVQkTHM8mHdmJS7iTSg8CjiWg3xaVTSGl/phHbMDby4T/IWPf
RZp2XTP+q+dC2JvKcIEwZpO8PnnhxAGPAJvFgU9E67rwqlNBaNs94TaIFXiq+ycO
OMoYJgwwJN1mmHRVLt6Mxus94fhcPiHITBV5SXfXAoGAShHWUp/Jpiqk5n+KAFI4
u5+3/1ZYuyHCip/nawweHVLcIvzXkN4Rw87EZP5s1YsjUVp0BwD/XXBcGOIkMJDd
RLSGq6CedBNOXGdV22XD1KTfh7IGXm/Wxhm85tvtn4/jXeAIu6P/NqOYdAL+a50P
m9ZQJAl2gVfJRbLjpK1ajUkCgYEAwT8VcWd85QMJ6wN5EPWd/rugowySBxstwT0R
sLS2uG3bbH0RWwj+aHkEvVsOlGhr5ZUO1M7kITh09/5IYokhUZUS8/VHL8koo0Ne
e17UD95lGcmW+pcUaS7TyQoxccARgHhoAB6P3XbxExEn/InSBKblovP6qQ7BizwJ
SYsvBcUCgYBkYFL8NzgoAbvYhBkKGclqFygWhc2Uarft8XCX7snNjmqjcg7SJL5X
STmBALNT39oRJNBZ+/5LQmc8lkLIfmke/xZ05rTmXuOLly7NUzHenLuauG/Cj7Kk
FH24lRGI2Qu/nXM59PmkcCfuPiSqrdt6JcW3jd8HoitDSxIDPZlNBw==
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
