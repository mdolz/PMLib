# -*- coding: utf-8 -*-

#======================================================================
# Counter class
#======================================================================

import daemon
import threading
import settings
import struct
import sys
import logging
import time
from daemon.Server import *
from math import ceil

class Info(threading.Thread):

    ## Constructor from Info class
    #
    #  It starts internal variables and start the thread
    #
    # @param [in] client         Socket that allows to communicate with client
    # @param [in] address        Address of client
    # @param [in] msg_type       Message type
    #
    def __init__(self, client, address, msg_type):

        self.client= client
        self.address= address
        self.msg_type= msg_type
        self.logger= logging.getLogger('pmlib_server.Info')  
        threading.Thread.__init__(self)
        self.start()

    ## Returns a packed integer
    #
    # @param [in] r        Integer to be packed
    # @param [ou] p        Integer packed
    #
    def pack_int(self, r):
        return struct.pack("i", int(r))

    ## Receives data from client
    #
    #  If length of message is 0 it indicates that client has closed the connection!
    #
    # @param [in] datatype        Type of data that we want to receive
    # @param [ou] p               Received data
    #
    def receive_data(self, datatype):
	msg=self.client.recv(struct.calcsize(datatype))
        if len(msg) == 0: 
            self.client.close()
            self.logger.error("Connection closed from client!: %s" % (repr(self.address)))
            sys.exit(1)
        return struct.unpack(datatype, msg)[0]

    ## Recevices a string from client
    #
    # @param [in] length        Length of string we want to receive
    # @param [ou] p             Received string
    #
    def receive_str(self, length):
        return self.client.recv(length)

    ## Send all data to client
    #
    # @param [in] msg        Message that we want to send to client
    #
    def send_all_data(self, msg):
        try:	 
            sent = self.client.sendall(msg)
        except:
            self.client.close()
            # self.logger.error("Connection closed from client!: %s" % (repr(self.address)))
            sys.exit(0)

#        totalsent = 0
#        while totalsent < len(msg):
#            try:
#                sent = self.client.send(msg[totalsent:])
#                if sent == 0:
#                   self.client.close()
#                   self.logger.error("Cannot set data from client!: %s" % (repr(self.address)))
#                   sys.exit(1)
#            except:
#                   self.client.close()
#                  # self.logger.error("Connection closed from client!: %s" % (repr(self.address)))
#                   sys.exit(0)
#            totalsent = totalsent + sent

    ## list_devices function is executed when client calls to pm_get_devices()
    #
    #  It returns data when a LIST_DEVICES message is received
    #
    def list_devices(self):
        msg= struct.pack("i", len(settings.devices_) )
        for dev in settings.devices_.values():
            msg+= struct.pack("i", len(dev.name) )
            msg+= dev.name
        self.send_all_data(msg)
     
    ## info_device function is executed when client calls to pm_info_device()
    #
    #  It returns data when a INFO_DEVICE message is received
    #
    # @param [in] client     Socket where we want to receive data
    #
    def info_device(self):
        dev_len= self.receive_data("i")
        name= self.receive_str(dev_len)
        if name in settings.devices_:
            msg=  struct.pack("i", settings.devices_[name].max_frequency )
            msg+= struct.pack("i", len(settings.devices_[name].lines) )
            msg+= struct.pack("i", len(settings.devices_[name].name) )
            msg+= settings.devices_[name].name
            self.send_all_data(msg)
        else:
            self.logger.error("Device name '%s' do not exist!" % (name))
            self.send_all_data(struct.pack("i", -1 ))

    ## info_device function is executed when client calls to pm_info
    #
    #  It returns data when a CMD_STATUS message is received
    #
    # @param [in] client     Socket where we want to receive data
    #
    def cmd_status(self):

        msg= struct.pack("i", len(settings.devices_) )
        self.send_all_data(msg)
        for dev in [ settings.devices_[i] for i in sorted(settings.devices_.keys()) ]:
            msg= struct.pack("i", len(dev.name) )
            msg+= dev.name
            msg+= struct.pack("i", dev.max_frequency )
            msg+= struct.pack("i", len(dev.lines) )

            pdu= False
            if isinstance(dev, daemon.devices.PDUDevice) or \
               isinstance(dev, daemon.devices.LMG450Device):
                pdu= True
                msg+= struct.pack("i", 1 )
            else:
                msg+= struct.pack("i", 0 )
                msg+= struct.pack("i", len(dev.computer.name) )
                msg+= dev.computer.name

            for lin in [ dev.lines[ii] for ii in sorted(dev.lines.keys()) ] :
                msg+= struct.pack("i", lin.number )
                msg+= struct.pack("i", len(lin.name) )
                msg+= lin.name
                msg+= struct.pack("f", lin.voltage )
                msg+= struct.pack("i", len(lin.description) )
                msg+= lin.description
                if pdu:
                    msg+= struct.pack("i", len(lin.computer.name) )
                    msg+= lin.computer.name                    

            msg+= struct.pack("i", len(dev.counters) )
            for con in [ dev.counters[ii] for ii in sorted(dev.counters.keys()) ] :
                msg+= struct.pack("i", id(con) )
                msg+= struct.pack("i", len(repr(con.address)) )
                msg+= repr(con.address)
                msg+= struct.pack("i", len(con.lines) )
                msg+= struct.pack("i" * len(con.lines), *con.lines )

            self.send_all_data(msg)

    ## info_device function is executed when client calls to pm_info
    #
    #  It returns data when a CMD_STATUS message is received
    #
    # @param [in] client     Socket where we want to receive data
    #
    def read_device(self):

        dev_name = self.client.recv( self.receive_data("i") )
        frequency= self.receive_data("i")

        if dev_name not in settings.devices_:   
            self.send_all_data(self.pack_int(-1))
            return

        dev= settings.devices_[dev_name]
        if frequency > dev.max_frequency:
            self.send_all_data(self.pack_int(-2))
            self.send_all_data(struct.pack("i", dev.max_frequency))
            return

        self.send_all_data(self.pack_int(0))

        while True:
            msg=  struct.pack("i", len(dev.sample))
            msg+= struct.pack("d" * len(dev.sample), *dev.sample)
            self.send_all_data(msg)

            if not self.receive_data("i"): break
            if frequency > 0:
                time.sleep(1.0/frequency)
            else:
                time.sleep(1.0/dev.max_frequency)

    ## Run function. Used to receive data of a client counter
    def run(self):
        if  self.msg_type == operations["LIST_DEVICES"]:
            self.list_devices()
        elif self.msg_type == operations["INFO_DEVICE"]:
            self.info_device()
        elif self.msg_type == operations["CMD_STATUS"]:
            self.cmd_status()
        elif self.msg_type == operations["READ_DEVICE"]:
            self.read_device()
        else:
            self.logger.error("Msg type %d could not be recognized: %s : %s" % (msg_type, self.device_name, str(e)))
            self.send_all_data(self.pack_int(-1))
        self.client.close()
        sys.exit(0)



