//////MEGA2560
#include <SPI.h>
#include <SD.h>
#include <arduino.h>
#include <MusicPlayer.h>

int Interrupt = 2;
int keypin = 21;
int ledpin = 49;

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  pinMode(ledpin, OUTPUT);
  pinMode(keypin, INPUT);
  attachInterrupt(Interrupt, stateChange, LOW);

  player.begin();
}
bool recstate = 0;
void stateChange()
{
  delay(5);
  if (digitalRead(keypin) == HIGH)
  {
    delay(5);
    if (digitalRead(keypin) == HIGH)
    {
      recstate = !recstate;
      digitalWrite(ledpin, recstate);
      if (recstate)
      {
        player.opRecord();
      }
      else
      {
        player.opStopRecord();
      }
    }
  }
}

byte data[32];
int index = 0;

void loop() {
  if (Serial2.available()&&!recstate)
  {
    if (index == 32)
    {
      vs1053.writeData(data, 32);
      index = 0;
    }
    data[index] = Serial2.read();
    index++;
  }
  player.play();

}
