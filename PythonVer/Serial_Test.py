import serial  

  
ser = serial.Serial()
ser.baudrate = 9600  
ser.port = 1  
ser.open()  
print ser.portstr  
strInput = raw_input('enter some words:')  
n = ser.write(strInput)  
print n  
strw = ser.read(n)  
print strw 
