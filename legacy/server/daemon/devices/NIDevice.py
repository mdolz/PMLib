# -*- coding: utf-8 -*-

#======================================================================
# NIDevice class
#======================================================================

import Device
import socket
import sys
import struct
import time

## A National Instruments device description
#
class NIDevice(Device.AttachedDevice):

    ## Creates a National Instruments device description and adds it
    ## to the devices dictionary
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] computer       The computer the device is attached to
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, computer, url, max_frequency):
	self.n_lines = 1
	self.r_lines = 32
	self.server_ip = '150.128.81.36'
	self.server_port = 4322
        super(NIDevice, self).__init__(name, computer, url, max_frequency)
        self.lines[0]= Device.Line(0, "Main", 12, "Main line")

	f= open("linear_adjust.txt", "r")
	l= f.readlines()
	self.lines_adjust= {}
	for i in xrange(self.r_lines): 
	    self.lines_adjust[i]= [0.0, 0.0]
	for i in l:
	    [id, a, b]= i.split("\t")
	    self.lines_adjust[int(id)]= [float(a), float(b)]
	
    def add_line(self, number, name, voltage, description=""):
	#"""
        if self.lines.has_key(number):
            msg="there are at least two lines with the same name, '{0}', in device '{1}'.".format(number, self.name)
            raise SyntaxError, msg
        self.lines[number]=Device.Line(number, name, voltage, description)
	#"""       

    def transf(self, v, i):
	return self.lines_adjust[i][0] * v + self.lines_adjust[i][1]

    def read(self):
#	try:	
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	#	except socket.error, msg:
	#  	sys.stderr.write("[ERROR] %s\n" % msg[1])
  	#	sys.exit(1)
 
#	try:
  	sock.connect((self.server_ip, self.server_port))
	#	except socket.error, msg:
  	#	sys.stderr.write("[ERROR] %s\n" % msg[1])
  	#	sys.exit(2)

	power= [0] * self.n_lines

	#"""
        voltages= [0] * 32
	voltages[0]= 12
	voltages[1]= 12
	voltages[2]= 12
	voltages[3]= 12
	voltages[4]= 12
	voltages[5]= 12
	voltages[6]= 12
	voltages[7]= 12
	#voltages[8]= 12
	#voltages[9]= 12
	#"""

	d_size= struct.calcsize('d')

	count= 0
        while self.running:
		
		msg = sock.recv(self.r_lines * d_size)
		while len(msg) < (self.r_lines * d_size):
			msg+= sock.recv(self.r_lines * d_size - len(msg))
		
		received = struct.unpack(self.r_lines * 'd', msg)

		if count == 0:
			power= 0
			for i in range(self.r_lines):
			#	power[i] = self.transf(received[i], i) * self.lines[i].voltage
				power+= self.transf(received[i], i) * voltages[i]
			count= 7
			yield [power]
			#yield power
		count-= 1

	sock.close()
