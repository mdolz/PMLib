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

class Counter(threading.Thread):

    ## Constructor from Counter class
    #
    #  It starts internal variables and start the thread
    #
    # @param [in] client         Socket that allows to communicate with client
    # @param [in] address        Address of client
    #
    def __init__(self, client, address):
        self.client= client
        self.address= address
        self.running= False
        self.logger= logging.getLogger('pmlib_server.Counter')  
        self.init_counter()
        threading.Thread.__init__(self)
        self.start()

    ## Start function is executed when client calls to pm_start_counter()
    #
    #  It starts the counter when a START message is received
    #
    def start_(self):
        if self.status == states["INACTIVE"] and self.last_set_completed():
            self.device.start_counter(self)
            self.status= states["ACTIVE"]
            self.sets= [[self.device.get_lines_length(self.lines), None]]
            return self.retval(0)
        else: 
            self.logger.error("Received <start> of an active counter or with not completed set! : %s" % (repr(self.address)))
            return self.retval(-1)

    ## Continue function is executed when client calls to pm_continue_counter()
    #
    #  It continues the counter when a CONTINUE message is received
    #
    def continue_(self):
        if self.status == states["INACTIVE"] and self.last_set_completed():           
            self.device.start_counter(self)
            self.status= states["ACTIVE"]
            self.sets.append([self.device.get_lines_length(self.lines), None])
            return self.retval(0)
        else: 
            self.logger.error("Received <continue> of an active counter or with not completed set! : %s" % (repr(self.address)))
            return self.retval(-1)

    ## Stop function is executed when client calls to pm_stop_counter()
    #
    #  It stops the counter when a STOP message is received
    #
    def stop_(self):
        if self.status == states["ACTIVE"] and not self.last_set_completed():
            self.device.stop_counter(self)
            self.status= states["INACTIVE"]
            self.sets[-1][1]= self.device.get_lines_length(self.lines)
            return self.retval(0)
        else:
            self.logger.error("Received <stop> of an inactive counter or with completed set! : %s" % (repr(self.address)))
            return self.retval(-1)

    ## Get function is executed when client calls to pm_get_counter()
    #
    #  It returns data of the counter when a GET message is received
    #
    def get_(self):        
        if self.status == states["INACTIVE"] and self.last_set_completed(): # and self.sets:
            watts_sets= [0]
            watts= []

            min_len= [0] * len(self.sets)            
            for i, se in enumerate(self.sets):
                for j, l in enumerate(self.lines):
                     if j == 0: min_len[i]= int(ceil((se[1][l] - se[0][l])/float(self.interval)))
                     else:      min_len[i]= int(min(ceil((se[1][l] - se[0][l])/float(self.interval)), min_len[i]))
                watts_sets.append(min_len[i] + watts_sets[-1])
 
            if self.aggregate:
                for i, se in enumerate(self.sets):
                     sum_line= [0] * min_len[i]
                     for j, l in enumerate(self.lines):
                          line= [self.device.lines[l].power_data[w] for w in xrange(se[0][l], se[1][l], self.interval)][:min_len[i]]       
                          for idx in xrange(min_len[i]):
                               sum_line[idx]+= line[idx]
                     watts.extend(sum_line)

            else:
                for j, l in enumerate(self.lines):
                     for i, se in enumerate(self.sets):
                          watts.extend([self.device.lines[l].power_data[w] for w in xrange(se[0][l], se[1][l], self.interval)][:min_len[i]])

           # mask= "".join([{False: "0", True: "1"}[i in self.lines] for i in xrange(128)])
           # smask= "".join([unichr(int(mask[i:i+8], base=2)) for i in xrange(0, len(mask), 8)])
         
           # print "Watts: ", len(watts)
           # print "Watts sets: ", len(watts_sets)
           # print "Watts sets: ", watts_sets
           # print "Watts mask: ", len(smask)
           # print "Watts mask: ", smask
           # print "Watts: ", watts

            msg=  struct.pack("i", len(watts) )
            msg+= struct.pack("i", len(watts_sets) )
            msg+= struct.pack("i", len(self.lines) )
            msg+= struct.pack("i" * len(watts_sets), *watts_sets )
            msg+= struct.pack("d" * len(watts), *watts )

            return msg
        else: 
            self.logger.error("Received <get> of an active counter or with not completed set! : %s" % (repr(self.address)))
            return self.retval(-1)

    ## Finalize function is executed when client calls to pm_finalize_counter()
    #
    #  It finalizes/destroys the counter when a FINALIZE message is received
    #
    def finalize_(self):
        self.destroy()
        return self.retval(0)

    ## Destroy function is used to destroy the counter whenever client
    ## destroys counter or closes accidentally its connection
    def destroy(self):
        if self.status == states["ACTIVE"]:
            self.device.stop_counter(self)
        self.device.remove_counter(self)
        self.running= False

    ## Check if last set is completed
    #
    # @param [ou] True/False        Return True if last set is completed
    #
    def last_set_completed(self):
	if self.sets == []: return True
	else: return self.sets[-1][1] != None

    ## Decode and sets mask line received from client
    def set_lines(self, lines):
        self.lines= []
        mask= bin(reduce(lambda x, y : (x<<8)+y, (ord(c) for c in lines), 1))[3:]
        nmask= "".join([mask[i:i+8][::-1] for i in xrange(0, len(mask), 8)])
        nmask= nmask[:len(self.device.lines)]
        for i,l in enumerate(nmask):
            if bool(int(l)): self.lines.append(i)
        if not self.lines:
            self.lines= [i for i in xrange(len(self.device.lines))]

    ## Returns a packed integer
    #
    # @param [in] r        Integer to be packed
    # @param [ou] p        Integer packed
    #
    def retval(self, r):
        return struct.pack("i", int(r))

    def receive_all_data(self, data):
        try:
            msg=self.client.recv(data)
            if len(msg) == 0: 
               self.destroy()
               self.client.close()
               self.logger.error("Connection closed from client!: %s" % (repr(self.address)))
               sys.exit(0)
            return msg
        except Exception, e:
            self.logger.error("Data could not be received from client!: %s : %s" % (repr(self.address), str(e)))
            sys.exit(0)

    ## Receives data from client
    #
    #  If length of message is 0 it indicates that client has closed the connection!
    #
    # @param [in] datatype        Type of data that we want to receive
    # @param [ou] p               Received data
    #
    def receive_data(self, datatype):
        msg=self.receive_all_data(struct.calcsize(datatype))
        return struct.unpack(datatype, msg)[0]

    ## Recevices a string from client
    #
    # @param [in] length        Length of string we want to receive
    # @param [ou] p             Received string
    #
    def receive_str(self, length):
        return self.receive_all_data(length)

    ## Send all data to client
    #
    # @param [in] msg        Message that we want to send to client
    #
    def send_all_data(self, msg, retries= 0):
        try:
            sent= self.client.sendall(msg)
            if sent:
                self.logger.error("Data could not be sent from Counter class!")
                raise RuntimeError
        except Exception, e:
            self.logger.error("Data could not be sent from Counter class!: %s : %s" % (repr(self.address), str(e)))
            sys.exit(0)

    ## Counter initialization
    #
    #  When client creates a new counter this function is executed.
    #  It initializes all data for the new counter
    #
    def init_counter(self):
        try:
            self.status= states["INACTIVE"]
            self.sets= []
    
            # Receive length and name of device name
            length= self.receive_data("i")
            self.device_name= self.receive_str(length)

            if self.device_name not in settings.devices_:
                raise Exception

            self.device= settings.devices_[self.device_name]            
         
            self.frequency= self.receive_data("i")
            if self.frequency == 0:
                self.frequency= self.device.max_frequency
            elif self.frequency > self.device.max_frequency:
                raise Exception("Device only works at %d Hz!" % self.device.max_frequency)

            # Receive interval
            self.interval= int(round( self.device.max_frequency / float(self.frequency) ))
            # Receive if we want only aggregate power of active lines or not
            self.aggregate= bool(self.receive_data("i"))
            # Receive lines mask
            length= self.receive_data("i")
            self.set_lines(self.receive_str(length))

            # print "Dev: ", self.device_name
            # print "Int: ", self.interval
            # print "Agr: ", self.aggregate
            # print "Lin: ", self.lines
            
            # Add counter to dictionary
            self.device.add_counter(self)
            self.running= True

            # Response to the client
            self.send_all_data(self.retval(0))
            self.logger.info("Created new counter: %s : %s" % (repr(self.address), self.device_name))

        except Exception, e:
            self.logger.error("Error when creating new counter: %s : %s : %s" % (repr(self.address), self.device.device_name, str(e)))
            self.send_all_data(self.retval(-1))

    ## Run function. Used to receive data of a client counter
    def run(self):
        try:   
            while self.running:
		msg_type= self.receive_data("i")
                if   msg_type == operations["START"]:
                    v= self.start_()
                elif msg_type == operations["CONTINUE"]:
                    v= self.continue_()
                elif msg_type == operations["STOP"]:
                    v= self.stop_()
                elif msg_type == operations["GET"]:
                    v= self.get_()
                elif msg_type == operations["FINALIZE"]:
                    v= self.finalize_()
                else:
                    self.logger.error("Msg type %d could not be recognized: %s : %s" % (msg_type, self.device_name, str(e)))
                    v= self.retval(-1)
                self.send_all_data(v)
            self.client.close()
            sys.exit(0)
        except Exception, e:
            self.logger.error("Error in self.run() of Counter class: %s : %s : %s" % (repr(self.address), self.device_name, str(e)))
            self.client.close()
            sys.exit(-1)



