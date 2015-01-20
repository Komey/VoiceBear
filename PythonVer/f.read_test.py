import os,sys

aaaaa = open('1.txt', 'rb')
index = 0
size = size=os.path.getsize("1.txt")
while(index<size):
    print aaaaa.read(32)
    index +=32
