# -*- coding: utf-8 -*-

#======================================================================
# PDUDevice class
#======================================================================

import Device
import pexpect
import time

## A PDU device description
#
class DummyDevice(Device.Device):

    ## Creates a PDU device description and adds it to the devices
    ## dictionary
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, url, max_frequency):
        super(DummyDevice, self).__init__(name, url, max_frequency)
        self.outlets= 24
        url= self.url.split("/")
        if url[0] != "ssh:":
            msg="daemon is only able to read power data from PDU through ssh protocol"
            raise SyntaxError, msg
        try:
            userpass, self.PDUname= url[-1].split("@")
            self.user, self.password= userpass.split(":")
        except Exception, e:
            raise SyntaxError, "PDU url is not formed properly", e
	

    ## Adds a pdu line description to the device
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
            msg="there are at least two lines with the same name, '{0}', in device '{1}'.".format(number, self.number)
            raise SyntaxError, msg
        self.lines[number]=Device.PDULine(number, name, computer, voltage, description)
        computer.add(self)


    def read(self):
    ##  Recoge los datos de consumo leyendo el puerto correpondiente
    ##  Lee del dispositivo de medida DC de todos los canales        

        power= [i/100. for i in xrange(1,self.outlets+1)]

        while self.running:        
            ini= time.time()

            for i in xrange(self.outlets):
               power[i]+= 1
            
            yield power

            sleep_time= 1 - (time.time() - ini)
            if sleep_time > 0: time.sleep(sleep_time)

