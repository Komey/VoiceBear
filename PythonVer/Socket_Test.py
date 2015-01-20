#!/usr/bin/env python
import socket
sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.bind(('',8845))
sock.listen(5)
while True:
	connection,address = sock.accept()
	print "client ip is "
	print address
	try:
		connection.settimeout(5)
		buf = connection.recv(1024)
		print buf
	except socket.timeout:
		print 'time out'
	connection.close()
