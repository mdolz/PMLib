# -*- coding: utf-8 -*-

#======================================================================
# DCDevice class
#======================================================================

import Device
import select
import termios
import serial
import time 
import sys

## A DCMeter device description
#
class ArduPowerDevice(Device.AttachedDevice):

	## Creates a DC2Meter device description and adds it to the
	## devices dictionary
	#
	# @param [in] name           The device name (used for identification, must be unique)
	# @param [in] computer       The computer the device is attached to
	# @param [in] url            The url of this device
	# @param [in] max_frequency  The maximum sample frequency of the device
	#
	def __init__(self, name, computer, url, max_frequency):
		self.n_lines = 0
		self.url = url
		super(ArduPowerDevice, self).__init__(name, computer, url, max_frequency)


	## Read function
	#
	#  Reads data from DCDevice, pyserial package is needed in order to run
	#
	def read(self):
		self.n_lines = len(self.lines)

		ser = serial.Serial(self.url, 115200, timeout=None) # Start serial interface
		time.sleep(1) # Wait 1 second before first serial interface usage

		power = [0] * self.n_lines # Initialise return list.
		channel_mask = 0 # Fill channel bitmask with zeros.
                pos = []
		 
		## Iterate over enabled & active channels and set corrosponding bit in channel_mask.
		for i in self.lines:
			channel_mask |= (1 << self.lines[i].number)
                        pos.append(self.lines[i].number)

		## Cut channel_mask into two seperate bytes.
		channel_mask_byte_LSB = channel_mask & 0xff
		channel_mask_byte_MSB = channel_mask >> 8

		ser.flushInput() # Flush serial input to be sure that theres no old data.

		## Write the channel bitmask to the serial interface (MSB byte first).
		ser.write(bytearray([channel_mask_byte_MSB, channel_mask_byte_LSB]))

		## Calculate positions array
		def positionss(channels):

			positions = [0] * channels

			for i in range(channels):
				p = 3 * (i/2) + 2

				if(i == channels-1 and i % 2 == 0):
					p -= 1
				
				positions[i] = p

			return positions


		## calculate shifts array
		def shifts(channels):

			shifts = []

			for i in range(channels/2):
				shifts.append(4)
				shifts.append(7)

			if(channels % 2 == 1):
				shifts.append(7)

			return shifts
			
		positions = positionss(self.n_lines)
		shifts = shifts(self.n_lines)
		n_bytes = positions[len(positions) - 1] + 1

		while self.running:

			lines = ser.read(size = n_bytes)

			var = True
			var2 = False

			if(ord(lines[0]) >> 7 == 0):
				for k in range(n_bytes):
					if(ord(lines[k]) >> 7 == 1):
						lines = lines[k:] + ser.read(size = k)
						var = False
						break
				
				if(var):
					continue
					
			for k in range(n_bytes - 1):
				if(ord(lines[k + 1]) >> 7 == 1):
					var2 = True
					break
					
			if(var2):
				continue
				
			
			j = 0
			for i in range(self.n_lines):
				value = ((ord(lines[positions[i]]) << shifts[i]) & 0x0380) | (ord(lines[j]) & 0x7F)

				j += 1

				if((i + 1) % 2 == 0 and i != 0):
					j += 1

				power[i] = ( ( float(value) - self.lines[pos[i]].offset ) / self.lines[pos[i]].slope ) * self.lines[pos[i]].voltage

			yield power

		ser.write(bytearray([0x00, 0x00]))














