/////LED Pin 13
/////subtopic = "light";
/////pubtopic = "con";

#define MQTTCLIENT_QOS2 1

#include <SPI.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <Ethernet.h>
#include <EthernetStack.h>


char printbuf[100];


int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  char _printbuf[100];
  sprintf(_printbuf, "Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n",
          ++arrivedcount, message.qos, message.retained, message.dup, message.id);
  Serial.print(_printbuf);
  sprintf(_printbuf, "Payload %s\n", (char*)message.payload);
  Serial.print(_printbuf);

  chled((char*)message.payload);

}


EthernetStack ipstack;
MQTT::Client<EthernetStack, Countdown> client = MQTT::Client<EthernetStack, Countdown>(ipstack);


const char* topic = "light";
const char* pubtopic = "con";

void connect()
{

  char hostname[] = "lot.XXXX.cn";////////////设置自己的MQTT服务器地址
  int port = 6666;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }

  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"Komey's Arduino";
  rc = client.connect(data);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");

  rc = client.subscribe(topic, MQTT::QOS2, messageArrived);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT subscribe is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT subscribed");
}


uint8_t mac[6] = {0x74, 0x65, 0x69, 0x2D, 0x30, 0xA1};
const int ledPin =  13;
int LEDState = 1;
void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  Ethernet.begin(mac);
  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());

  Serial.println("light MQTT demo");
  connect();
}
void chled(char * ms)
{
  if (ms[0] == '0')
  {
    LEDState = 0;
  }
  if (ms[0] == '1')
  {
    LEDState = 1;
  }
  MQTT::Message message;

  arrivedcount = 0;

  // Send and receive QoS 0 message
  char buf[100];
  sprintf(buf, "State", LEDState);
  Serial.println(buf);
  message.qos = MQTT::QOS2;
  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;
  message.payloadlen = strlen(buf) + 1;
  int rc = client.publish(pubtopic, message);
  while (arrivedcount == 0)
    client.yield(1000);

}
void doled()
{
  if (LEDState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  }
  else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
}
void loop()
{
  if (!client.isConnected())
    connect();
  client.yield(1000);
  doled();
}
