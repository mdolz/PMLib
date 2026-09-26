# -*- coding: utf-8 -*-

#======================================================================
# Device class
#======================================================================

from daemon.modules.Computer import *
from daemon.modules.Counter import *
import threading
import logging

devices_= {}

## A device description
#
class Device(threading.Thread):
    
    ## Creates a device description and adds it to the devices
    ## dictionary
    #
    #  Before adding the given device description to the devices
    #  dictionary, it checks that the name of the new device has not
    #  been used by a previously added device.
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, url, max_frequency):
        threading.Thread.__init__(self)
       # self.daemon= True
        self.name=name
        self.url=url.replace("file:/", "")
        self.max_frequency=max_frequency
        self.lines={}
        if devices_.has_key(name):
            msg="there are at least two devices with the same name ({0}).".format(self.name)
            raise SyntaxError, msg
        # Register the device
        devices_[name]= self
        
        self.running= True
        self.working= False
        self.ko= True
        self.counters= {}
      #  self.active_counters= 0
        self.sample= [0.0] * self.n_lines
        self.start_condition= threading.Event()
        self.polling_condition= threading.Event()
        self.counter_lock= threading.Lock()
        self.logger= logging.getLogger('pmlib_server.Device')

    ## Adds a line description to the device
    #
    #  Before adding the given line description to the device, it
    #  checks that the name of the new line has not been used by a
    #  previously added line.
    #
    # @param [in] name        The line name (used for identification)
    # @param [in] voltage     The line voltage
    # @param [in] description A text description of the line
    #
    def add_line(self, number, name, voltage, description, slope= 0, offset= 0):
        if self.lines.has_key(number):
            msg="there are at least two lines with the same name, '{0}', in device '{1}'.".format(number, self.number)
            raise SyntaxError, msg
        self.lines[number]= Line(number, name, voltage, description, slope, offset)

    ## Appends a sample to the lines when device reads a new one
    #
    #  Before adding a new sample to the lines it checks if 
    #  line is enabled, i.e., if a counter has requested that line.
    #
    # @param [in] sample         The new sample
    #
    def line_append_data(self, sample):
        for line_id, l in self.lines.iteritems():
            if l.is_active():
                l.power_data.append(sample[line_id])

    ## Construct a dictionary with the lengths of lines as keys
    #
    #  The dictionary contains the lenghts of the data arrays of
    #  lines, indexed by number line
    #
    # @param [in] lines         List of id lines
    # @param [ou] lines_length  Dictionary with the lengths of lines
    #
    def get_lines_length(self, lines):
        r= {}
	for l in lines:
	    r[l]= len(self.lines[l].power_data)
        return r
       
    ## Queries if the device has any associated counter
    #
    #  The dictionary contains the lenghts of the data arrays of
    #  lines, indexed by number line
    #
    # @param [ou] True/False    Returns true if device has counters
    #
    def has_counters(self):
        return len(self.counters) > 0

    ## Register a new counter
    #
    #  When new counter is created, it adds itself to the
    #  device counter dictionary. This function also enables
    #  requested lines into the device.
    #
    # @param [in] c       Counter object
    #
    def add_counter(self, c):
        self.counter_lock.acquire()
        self.counters[id(c)]= c
        for l in c.lines:
            self.lines[l].enable()
        self.counter_lock.release()

    ## Delete a counter
    #
    #  When new counter is finished, it deletes itself of the
    #  device counter dictionary. This function also disables
    #  counter lines into the device and flushes data from arrays.
    #
    # @param [in] c       Counter object
    #
    def remove_counter(self, c):
        self.counter_lock.acquire()
        del self.counters[id(c)]
        for l in c.lines:
            self.lines[l].disable()
            if not self.lines[l].is_enabled():
                self.lines[l].power_data= []
#		print "CLEAN DATA"
        self.counter_lock.release()

    def start_counter(self, c):
        self.counter_lock.acquire()
        for l in c.lines:
            self.lines[l].activate()
        self.counter_lock.release()
 
    def stop_counter(self, c):
        self.counter_lock.acquire()
        for l in c.lines:
            self.lines[l].inactivate()
        self.counter_lock.release()

    ## Run function. Used to receive data from devices
    #
    #  First it checks if all needed lines are defined in settings.py
    #  Then, it starts polling data from device, and appends it to
    #  data lines array it needed.
    #
    def run(self):
        if len(self.lines) < self.n_lines:
            self.ko= True
            self.start_condition.set()
            raise SyntaxError("There are not enough lines defined, device %s supports %d lines, only defined %d!" % (self.name, self.n_lines, len(self.lines)))
#       try:
        for d in self.read():
                if not self.working:
                    self.working= True
                    self.start_condition.set()
                self.sample= d
                self.line_append_data(d)
              #  self.polling_condition.wait(timeout= 10)

        #except Exception, e:
        #    self.ko= True
        #    self.start_condition.set()
        #    raise IOError("Can't read data from usb device %s: %s" % (self.url, str(e)))

    ## Stop function. Used to stop thread from running state.
    #
    #  It makes changes running condition to False, and
    #
    def stop(self):
        self.running= False
    #    self.polling_condition.set()


## A device attached to a computer description
#
class AttachedDevice(Device):
    
    ## Creates device attached to a computer description and adds it
    ## to the devices dictionary
    #
    #  Before adding the given device description to the devices
    #  dictionary, it checks that the name of the new device has not
    #  been used by a previously added device.
    #
    # @param [in] name           The device name (used for identification, must be unique) 
    # @param [in] computer       The computer the device is attached to
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, computer, url, max_frequency):
        #print type(computer), type(daemon.modules.Computer.Computer)
        if not isinstance(computer, daemon.modules.Computer):
            msg="the given computer parameter is not a Computer object"
            raise SyntaxError, msg
        self.computer= computer
        super(AttachedDevice, self).__init__(name, url, max_frequency)
        # Register the device in the computer it is attached to
        self.computer.add(self)


## A line description
#
class Line(object):

    ## Creates a line description
    #
    # @param [in] name        The line name (used for identification)
    # @param [in] voltage     The line voltage
    # @param [in] description A text description of the line
    #
    def __init__(self, number, name, voltage, description, slope= 0, offset= 0):
        self.number= number
        self.name= name
        self.voltage= voltage
        self.description= description
        self.power_data= []
        self.enabled= 0
        self.active= 0
        self.slope= slope
        self.offset= offset

    ## Returns a string representation for this line
    def __repr__(self):
        return "Line {0} (name: {1}, voltage: {2}, description: '{3}', slope: {4}, offset: {5})".format(self.number, self.name, self.voltage, self.description, self.slope, self.offset)

    ## Enables a line
    def enable(self):
        self.enabled+= 1

    ## Disables a line
    def disable(self):
        self.enabled-= 1
        if self.enabled < 0:
            self.enabled= 0

    ## Check if line is enabled
    #
    # @param [ou] True/False         Return true if line is enabled
    #
    def is_enabled(self):
        return self.enabled > 0

    ## Enables a line
    def activate(self):
        self.active+= 1

    ## Disables a line
    def inactivate(self):
        self.active-= 1
        if self.active < 0:
            self.active= 0

    ## Check if line is enabled
    #
    # @param [ou] True/False         Return true if line is enabled
    #
    def is_active(self):
        return self.active > 0


## A PDU line description
#
class PDULine(Line):

    ## Creates a PDU line description
    #
    # @param [in] name        The line name (used for identification)
    # @param [in] computer    The computer this PDU line is attached to
    # @param [in] voltage     The line voltage
    # @param [in] description An optional text description of the line
    #
    def __init__(self, number, name, computer, voltage, description=""):
        if not isinstance(computer, daemon.modules.Computer) and computer != None:
            msg="the given computer parameter is not a Computer object"
            raise SyntaxError, msg
        super(PDULine, self).__init__(number, name, voltage, description)
        if computer:
            self.computer= computer
        else:
            self.computer= daemon.modules.Computer("NULL", "NULL")

    ## Returns a string representation for this line
    def __repr__(self):
        return "Line {0} (name: '{1}', computer: '{2}', voltage: {3}, description: '{4}')".format(self.number, self.name, self.computer.name, self.voltage, self.description)


