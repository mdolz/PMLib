# -*- coding: utf-8 -*-

#======================================================================
# DCDevice class
#======================================================================

import Device
import select
import termios
#import serial
import time 

## A DCMeter device description
#
class DCDevice(Device.AttachedDevice):

    ## Creates a DC2Meter device description and adds it to the
    ## devices dictionary
    #
    # @param [in] name           The device name (used for identification, must be unique)
    # @param [in] computer       The computer the device is attached to
    # @param [in] url            The url of this device
    # @param [in] max_frequency  The maximum sample frequency of the device
    #
    def __init__(self, name, computer, url, max_frequency):
        self.n_lines= 12
        super(DCDevice, self).__init__(name, computer, url, max_frequency)

    ## Read function
    #
    #  Reads data from DCDevice, pyserial package is needed in order to run
    #
    def read(self):
    ##  Recoge los datos de consumo leyendo el puerto correpondiente
    ##  Lee del dispositivo de medida DC de todos los canales    

        fd = serial.Serial(port=self.url , \
                            baudrate=19200, \
                            rtscts= True, \
                            dsrdtr= True, \
                            timeout= 1, \
                            parity= serial.PARITY_ODD )

        current     = [0] * self.n_lines
        power       = [0] * self.n_lines

        cnt= 10
#        t= time.time()
#        c= 0
        while self.running:
            sample = fd.readline().strip(" \n\t\r").split(';')[:-1]
            if cnt >0:
                cnt-= 1
                continue
            if len(sample) == self.n_lines:
                for i in range(self.n_lines):
                    current[i] = int(sample[i], base = 16) * 3.051804E-4
                    try:
                        power[i] = current[i] * self.lines[i].voltage
                    except:
                        power[i] = current[i] * 12

#                if time.time()-t>= 1.0:
#                        print "Freq: ", c
#                        t= time.time()
#                        c= 0
#                c+= 1

                yield power

        fd.close()

