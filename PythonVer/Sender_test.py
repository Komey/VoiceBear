import serial  
import time
import os

filename = "2.mp3"

f = open(filename,'rb')
size=os.path.getsize(filename)
print size
print "Start Play!"

ser = serial.Serial()
ser.baudrate = 9600  
ser.port = 1  
ser.open()  
print ser.portstr

index = 0;
while(index < size):
    ser.write(f.read(32))
    index +=32

