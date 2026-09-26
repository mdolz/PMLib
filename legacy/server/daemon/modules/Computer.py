# -*- coding: utf-8 -*-

#======================================================================
# Computer class
#======================================================================

import daemon

## Dictionary of computers
computers={}

## A computer description
#
class Computer(object):

    ## Creates a computer description and adds it to the computers
    ## dictionary
    #
    # @param [in] name  The name of the computer
    # @param [in] ip    The IP address of the computer
    #
    def __init__(self, name, ip):
        self.name=name
        self.ip=ip
        self.devices={}
        # Register the computer
        computers[name]=self
        
    ## Returns a string representation for this computer
    def __repr__(self):
        return "Computer {0} ({1}): {2} device(s)".format(self.name, self.ip, len(self.devices))

    ## Adds a device description to the computer
    #
    # @param [in] device  A device description object
    #
    def add(self, device):
        #if not isinstance(device, daemon.devices.Device):
        #    msg="the given device parameter is not a Device object"
        #    raise SyntaxError, msg
        self.devices[device.name]=device

