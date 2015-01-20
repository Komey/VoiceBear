import urllib2
url = 'http://7sbpnc.com1.z0.glb.clouddn.com/2.mp3' 
f = urllib2.urlopen(url) 
data = f.read() 
with open("demo2.zip", "wb") as code:     
    code.write(data)
