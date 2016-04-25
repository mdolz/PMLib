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
	self.n_lines = 32
	self.r_lines = 32
	self.server_ip = '150.128.81.36'
	self.server_port = 4322
        super(NIDevice, self).__init__(name, computer, url, max_frequency)
#        self.lines[0]= Device.Line(0, "Main", 12, "Main line")


    def add_line(self, number, name, voltage, description=""):
	#"""
        if self.lines.has_key(number):
            msg="there are at least two lines with the same name, '{0}', in device '{1}'.".format(number, self.name)
            raise SyntaxError, msg
        self.lines[number]=Device.Line(number, name, voltage, description)
	#"""       


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
	#"""

	count= 0
        while self.running:
		
		total_length = 0
		msg = sock.recv(self.r_lines*8)
		total_length = total_length + len(msg)
		
		while total_length < (self.r_lines*8):
			msg2 = sock.recv(self.r_lines*8-total_length)
			total_length = total_length + len(msg2)
			msg = msg + msg2
		
		#print '{0:.16f}'.format(time.time())
		'''
		try:
			received= struct.unpack(32*'d',msg)
		except:
			msg2 = struct.unpack(len(msg)/8 * 'd',msg)
			#print msg2
			if len(msg) < (self.n_lines*8):
				received = msg2 + struct.unpack((32-len(msg)/8)*'d', sock.recv((self.n_lines*8-len(msg)))) 
			#print 'received ', received
			#print
		'''
	
		received = struct.unpack(self.r_lines*'d',msg)

		if count == 0:
			#power= 0
			for i in range(self.r_lines):
				power[i] = received[i] * self.lines[i].voltage	
				#power+= received[i] * voltages[i]
			count= 7
			yield power
		count-= 1

	sock.close()
