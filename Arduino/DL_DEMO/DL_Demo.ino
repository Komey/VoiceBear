#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <HttpClient.h>


File myFile;

char * server = "XXXX.clouddn.com";///////七牛的下载地址
char * kPath = "/1.mp3";

char* Filename = "2.MP3";

uint8_t mac[6] = {0x74,0x65,0x69,0x2D,0x30,0x38};
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;


void showfiles()
{
  File root;
  root = SD.open("/");
  printDirectory(root, 0);
  
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
void checkfile(char* _file) {
  int ret = 0;
  delay(100);
  while (SD.exists(_file)) {
    Serial.println("file exists.");
    Serial.println("Removing file...");
    SD.remove(Filename);
    delay(100);
    ret++;
    if (ret > 10) {
      Serial.println("SD Error...");
    }
  }
  ret = 0;
  /////////////////////////////////////////////////
  Serial.print("File creating.");
  Serial.println(_file);
  while (!SD.exists(_file))
  {
    myFile.close();
    myFile = SD.open(_file, FILE_WRITE);
    myFile.close();
    Serial.print(".");
    delay(50);
    ret++;
    if (ret > 10) {
      Serial.println("SD Error...");
    }
  }
  Serial.println();
}


void setup() {
  Serial.begin(9600);
  
  while(!SD.begin(4))
  {
    Serial.println("Start SD Card! ");
  }
  showfiles();
  while(!Ethernet.begin(mac))
  {
  	Serial.println("Start Ethernet! ");
  }
  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  //////////////////////////////////////////////
  
  delay(1000);
}
void dlfile()
{
  int retrytime = 0;
  EthernetClient httpclient;
  HttpClient http(httpclient);
  Serial.println("connecting...");
  while (http.get(server, kPath)) {
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

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        unsigned long bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        checkfile(Filename);
        myFile = SD.open(Filename, FILE_WRITE);

        Serial.println("Downloading........");
        int bu_size = 1024;
        byte redbuf[bu_size];
        delay(100);
        unsigned long timeoutStart = millis();
        while ( (http.connected() || http.available()) &&
                ((millis() - timeoutStart) < kNetworkTimeout) )
        {

          if (http.available()) {
            if(bodyLen < bu_size)
            {
              http.read(redbuf,bodyLen);
              myFile.write(redbuf,bodyLen);
              bodyLen = 0;
            }
            else
            {
              http.read(redbuf,bu_size);
              myFile.write(redbuf,bu_size);
              bodyLen = bodyLen - bu_size;
            }
            timeoutStart = millis();
            //Serial.println(bodyLen);
            if(bodyLen==0)
            {
               http.stop();
               myFile.close();
            }
          }
          else
          {
            delay(kNetworkDelay);
          }
          
        }
        Serial.print("Download ok!.......");
        myFile.close();
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
  // if the server's disconnected, stop the client:
  Serial.println();
  Serial.println("disconnecting.");
  httpclient.stop();
  myFile = SD.open(Filename);
  Serial.println(Filename);
  Serial.println(myFile.size());
  
  // close the file:
  myFile.close();
  Serial.println("Done!");
  
  delay(1000);
  
}
int i = 0;
void loop() {

  if(i==0)
  {
    dlfile();
    i++;
  }
}

