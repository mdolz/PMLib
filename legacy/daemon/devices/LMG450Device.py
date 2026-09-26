# -*- coding: utf-8 -*-

#======================================================================
# LMG450Device class
#======================================================================

import Device
import time
import serial
import termios
import fcntl
import struct

## A LMG450 device description
#
class LMG450Device(Device.Device):

    ## Creates a LMG450 device description and adds it to the devices
    ## dictionary
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, url, max_frequency):
        self.n_lines= 4
        super(LMG450Device, self).__init__(name, url, max_frequency)

    ## Adds a LMG450 line description to the device
    #
    #  Before adding the given line description to the device, it
    #  checks that the name of the new line has not been used by a
    #  previously added line.
    #
    # @param [in] name        The line name (used for identification)
    # @param [in] computer    The computer the line is attached to
    # @param [in] voltage     The line voltage
    # @param [in] description An optional text description of the line
    #
    def add_line(self, number, name, computer, voltage, description=""):
        if self.lines.has_key(number):
            msg="there are at least two lines with the same name, '{0}', in device '{1}'.".format(number, self.name)
            raise SyntaxError, msg
        self.lines[number]=Device.PDULine(number, name, computer, voltage, description)
        if computer:
            computer.add(self)

    ## Read function
    #
    #  Reads data from PDUDevice, pexpect package is needed in order to run
    #
    def read(self):
    ##  Recoge los datos de consumo leyendo el puerto correpondiente
    ##  Lee del dispositivo de medida DC de todos los canales   

        fd = serial.Serial(port=self.url,
                                baudrate=57600,
                                bytesize=serial.EIGHTBITS,
                                rtscts= True,
                                stopbits= serial.STOPBITS_ONE,
#                               dsrdtr= True,  
                                timeout= 1 )

        fd.write(":SYSTem:LANGuage SHORT\n")

        fd.write("FRMT PACKED\n");
        fd.write("CYCL %.6f\n" % (1./self.max_frequency));
        fd.write("ACTN;P1?;P2?;P3?;P4?\n");
        fd.write("CONT ON\n");

        #power = [0] * self.n_lines
        sample= fd.readline()

        size= 7+1+4*struct.calcsize('f')
        while self.running:
            try:
                sample = fd.read(size)
            except Exception, e:
		continue
            f= struct.unpack('<1c1c1c4c4fc', sample)
            yield f[7:7+self.n_lines]

        fd.write("CONT OFF\n")
        fd.write("FRMT ASCII\n")
        fd.write("GTL\n")

        fd.close()

