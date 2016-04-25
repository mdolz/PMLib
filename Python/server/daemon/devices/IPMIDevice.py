# -*- coding: utf-8 -*-

#======================================================================
# WattsUpDevice class
#======================================================================

from daemon.devices.Device import *
import select
import termios
import tty

## A WattsUp device description
#
class WattsUpDevice(AttachedDevice):

    ## Creates a WattsUp device description and adds it to the devices
    ## dictionary
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] computer       The computer the device is attached to
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, computer, url, max_frequency):
        self.n_lines= 1
	super(WattsUpDevice, self).__init__(name, computer, url, max_frequency)
        self.lines[0]= Line(0, "Main", 220, "Main line")
   

    ## Fake adding of line description to the device
    #
    #  The WattsUp Device does not have lines. This method avoids the
    #  base class method silently been called.
    #
    # @param [in] name        The line name (used for identification)
    # @param [in] voltage     The line voltage
    # @param [in] description A text description of the line
    #
    def add_line(self, name, voltage, description):
        raise SyntaxError, "a WattsUp Device can not have lines"

    ## Read function
    #
    #  Reads data from WattsUp
    #z
    def read(self):
    ##  Recoge los datos de consumo leyendo el puerto correpondiente
    ##  Lee del dispositivo de medida wattsup

        power       = [0] * len(self.lines)
        sample      = ["0"] * len(self.lines)

        while self.running:
            system("")
            select.select([fd], [], [])
            sample= fd.readline().strip(" \n\t\r;").split(',')[3:]
            if len(sample) == 18:
                power[0] = float(sample[0]) * 1e-1
  	        yield power


