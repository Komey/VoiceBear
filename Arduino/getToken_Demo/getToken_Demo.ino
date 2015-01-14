#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>


byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

char tokenserver[] = "XXX.sinaapp.com";////////修改自己的token授权地址
char tokenurl[] = "/up_bear.php";

char uptoken[124];

const int kNetworkTimeout = 30 * 1000;
const int kNetworkDelay = 1000;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  getToken();
}


int getToken() {
  int tokenstate = 0;
  int retrytime = 0;
  EthernetClient httpclient;
  HttpClient http(httpclient);
  Serial.println("A token connecting...");
  while (http.get(tokenserver, tokenurl)) {
    delay(500);
    Serial.println("reconnecting...");
    retrytime++;
    if (retrytime > 10)
    {
      Serial.println("Failed to connect server!...");
      break;
    }
  }
  if (retrytime <= 10)
  {
    Serial.println("started Request ok");
    int err;
    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);
      if (err != 200)
      {
        Serial.print("Can't get token!");
        return 0;
      }
      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int nshows = 0;
        String _Token = "";
        unsigned long timeoutStart = millis();
        while ( (http.connected() || http.available()) &&
                ((millis() - timeoutStart) < kNetworkTimeout) )
        {
          if (http.available()) {
            char c = http.read();
            if (nshows == 1) {
              _Token += c;
              //Serial.print(c);
            }
            if (c == '\n') {
              nshows++;
            }
            timeoutStart = millis();
          }
          else
          {
            delay(kNetworkDelay);
          }
        }
        if(_Token.length()==124)
        {
          tokenstate = 1;
        }
        _Token.toCharArray(uptoken, _Token.length() );
        Serial.print("Get Token : ");
        Serial.println(uptoken);
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  http.stop();
  httpclient.stop();
  delay(1000);
  return tokenstate;
}

void loop() {
  delay(10000);
  while(!getToken())
  {
    Serial.println("Can't Get Token");
  }
  
}
