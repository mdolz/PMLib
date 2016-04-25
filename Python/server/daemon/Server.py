# -*- coding: utf-8 -*-

#======================================================================
# Server class
#======================================================================

import settings
import threading
import sys
import signal
import time
import socket
import struct
import logging
from termcolor import colored
from modules.Counter import *
from modules.Info import *

operations= { "CREATE"      : 0, \
              "START"       : 1, \
              "CONTINUE"    : 2, \
              "STOP"        : 3, \
              "GET"         : 4, \
              "FINALIZE"    : 5, \
              "INFO_DEVICE" : 6, \
              "LIST_DEVICES": 7, \
              "CMD_STATUS"  : 8, \
              "READ_DEVICE" : 9, \
              "ERROR"       : -1 }

states= { "INACTIVE": 0, \
          "ACTIVE"  : 1 }

## Server class
#
class Server:

    ## Server class, it is encharged to receive requestes from clients
    #
    #  Construcor initializes al needed data for working
    #
    def __init__(self):
        self.host= settings.IP
        self.clientport= settings.PORT
        self.clientsocket= False
        self.running = True
        signal.signal(signal.SIGINT, self.handler)
        signal.signal(signal.SIGHUP, self.handler)
        signal.signal(signal.SIGQUIT, self.handler)
        signal.signal(signal.SIGILL, self.handler)
        signal.signal(signal.SIGABRT, self.handler)
        signal.signal(signal.SIGFPE, self.handler)
        #signal.signal(signal.SIGKILL, self.handler)
        signal.signal(signal.SIGSEGV, self.handler)
        signal.signal(signal.SIGPIPE, self.handler)
        #signal.signal(signal.SIGALARM, self.handler)
        signal.signal(signal.SIGTERM, self.handler)
        signal.signal(signal.SIGUSR1, self.handler)
        signal.signal(signal.SIGUSR2, self.handler)
        signal.signal(signal.SIGCHLD, self.handler)
        signal.signal(signal.SIGCONT, self.handler)
        #signal.signal(signal.SIGSTOP, self.handler)
        #signal.signal(signal.SIGSTP, self.handler)
        #signal.signal(signal.SIGTIN, self.handler)
        #signal.signal(signal.SIGTOU, self.handler)
        self.logger= logging.getLogger('pmlib_server.Server')
	

    ## Stop handler is handles SIGINT signals and stops the server
    def handler(self, signum, frame):
        self.logger.info("Manejador (%s)\n" % (signum))
	self.stop()

    ## Writes to stdout and flushes
    #
    # @param [in] msg      Message to be printed
    #
    def stdout(self, msg):
        sys.stdout.write(msg); sys.stdout.flush()

    ## Stops the server
    #
    #  Stops all the devices and also the server itself
    #
    def stop(self):
        try:
            self.stdout("Stopping devices...\n")
            self.logger.info("Stopping devices")

            for d in [ settings.devices_[i] for i in sorted(settings.devices_.keys()) ]:
                if d.working:
                    self.stdout("Stopping device %s...%s" % (d.name, " " * (15-len(d.name))) )
                    d.stop()
                    self.stdout(colored("[  OK  ]\n", "green"))
                    self.logger.info("Device %s stopped" % (d.name))

            self.running= False

            self.stdout("Stopping server... %s"% (" " * (15)) )
            self.logger.info("Server stopped")
            if self.clientsocket:
                self.clientsocket.close()
            self.stdout(colored("[  OK  ]\n", "green"))

        except Exception, e:
            self.logger.error("Device could not be stopped! " + str(e))
            self.stdout(colored("[FAILED]\n", "red"))
            self.stdout(str(e))
        sys.exit(-1)

    ## Starts the server
    #
    #  Starts all the devices and also the server itself
    #
    def start_devices(self): 
        try:
            self.stdout("\n")
            self.stdout("Starting devices...\n")
            self.logger.info("Starting devices")

            for d in [ settings.devices_[i] for i in sorted(settings.devices_.keys()) ]:
                self.stdout("Starting device %s...%s" % (d.name, " " * (15-len(d.name))) )
                d.start()
                self.logger.info("Device %s started" % (d.name))

                d.start_condition.wait(timeout=15)

                if d.working:
                    self.stdout(colored("[  OK  ]\n", "green"))
                elif d.ko:
                    raise Exception

        except Exception, e:
            self.logger.error("Device could not be started! " + str(e))
            self.stdout(colored("[FAILED]\n", "red"))
            self.stop()

    ## Starts the server socket
    #
    #  Creates and start a TCP socket listening request in (host, port)
    #
    # @param [in] host       Host of the machine
    # @param [in] port       Port
    #
    def start_socket(self, host, port):
        #try:
        self.stdout("Starting server... %s"% (" " * (15)) )
        self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clientsocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.clientsocket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        self.clientsocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 120)
	self.clientsocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 120)
        self.clientsocket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 100)
        self.clientsocket.bind((host, port))
        self.clientsocket.listen(5)
        self.stdout(colored("[  OK  ]\n", "green"))
        self.stdout("Server listening at ('%s', %s)\n" % (host, port))
        self.logger.info("Server listening at ('%s', %s)\n" % (host, port))
        '''
        except Exception, e:
            self.logger.error("Server could not be started! " + str(e))
            self.stdout(colored("[FAILED]\n", "red"))
            self.stdout(str(e)) 
            self.stop()
        '''
    ## Receives data from client
    #
    # @param [in] client        Socket where we want to receive data
    # @param [in] datatype      Type of data that we want to receive
    # @param [ou] p             Received data
    #
    def receive_data(self, client, datatype):
        return struct.unpack(datatype, client.recv(struct.calcsize(datatype)))[0]

    ## Recevices a string from client
    #
    # @param [in] client        Socket where we want to receive data
    # @param [in] length        Length of string we want to receive
    # @param [ou] p             Received string
    #
    def receive_str(self, client, length):
        return client.recv(length)

    ## Send all data to client
    #
    # @param [in] client     Socket where we want to receive data
    # @param [in] msg        Message that we want to send to client
    #
    def send_all_data(self, client, msg):
        sent = client.sendall(msg)
        if sent:
            self.logger.error("Data could not be sent!")
            raise RuntimeError

#        totalsent = 0
#        while totalsent < len(msg):
#            sent = client.send(msg[totalsent:])
#            if sent == 0:
#                self.logger.error("Data could not be sent!")
#                raise RuntimeError
#            totalsent = totalsent + sent

    ## Returns a packed integer
    #
    # @param [in] r        Integer to be packed
    # @param [ou] p        Integer packed
    #
    def pack_int(self, r):
        return struct.pack("i", int(r))

    ## Run function. Used to receive request from clients
    #
    #  First it checks it start all devices and server socket
    #  Then, it starts waiting from client requests
    #
    def run(self):
        self.start_devices()
        self.start_socket(self.host, self.clientport)
        
        while self.running:
            try:
                client, address = self.clientsocket.accept()
            except Exception, e:
                self.logger.error("Error en accept: " +str(e))
 	    try:
                msg_type= self.receive_data(client, "i")
                if msg_type == operations["CREATE"]:
                    Counter(client, address)
                elif msg_type in [ operations["INFO_DEVICE"], operations["LIST_DEVICES"], operations["CMD_STATUS"], operations["READ_DEVICE"]]:
                    Info(client, address, msg_type)
                else:
                    client.send_all_data(pack_int(-1)) 
            except Exception, e:
                self.logger.error("Error in self.run() of Server class: " +str(e))
		continue

if __name__ == "__main__": 
    module_logger = logging.getLogger('pmlib_server')
    lg = logging.FileHandler(settings.LOGFILENAME)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    lg.setFormatter(formatter)
    module_logger.addHandler(lg)

    s= Server()
    s.run()
    
