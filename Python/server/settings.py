#======================================================================
# PowerMeter daemon settings
#======================================================================

from daemon.devices import *
from daemon.modules import *

#----------------------------------------------------------------------
# General section
#----------------------------------------------------------------------

# IP and Port in which the daemon will be listening (default: 6526)

IP="0.0.0.0"
PORT=6526

# Log file name (default: "/var/log/powermeter.log")
LOGFILENAME="/opt/pmlib/log/powermeter.log"
path_="/opt/pmlib"

#----------------------------------------------------------------------
# Computers section
#----------------------------------------------------------------------

amd1   = Computer(name="AMD-1",   ip="0.0.0.0")
amd2   = Computer(name="AMD-2",   ip="0.0.0.0")
amd3   = Computer(name="AMD-3",   ip="0.0.0.0")
amd4   = Computer(name="AMD-4",   ip="0.0.0.0")
amd5   = Computer(name="AMD-5",   ip="0.0.0.0")
intel1 = Computer(name="Intel-1", ip="0.0.0.0")
intel2 = Computer(name="Intel-2", ip="0.0.0.0")
intel3 = Computer(name="Intel-3", ip="0.0.0.0")
intel4 = Computer(name="Intel-4", ip="0.0.0.0")
intel5 = Computer(name="Intel-5", ip="0.0.0.0")
switch = Computer(name="Switch",  ip="0.0.0.0")
nas1   = Computer(name="NAS-1",   ip="0.0.0.0")
eeclust= Computer(name="eeclust", ip="0.0.0.0")

#----------------------------------------------------------------------
# Devices section
#----------------------------------------------------------------------

#lmg_0= LMG450Device(name="LMG450-0", url="/dev/ttyUSB2", max_frequency=20)
#lmg_0.add_line(number=0,  name="Outlet %d" % 1,  computer=eeclust,voltage=220, description="Outlet %d" % 1)
#lmg_0.add_line(number=1,  name="Outlet %d" % 2,  computer=intel5, voltage=220, description="Outlet %d" % 2)
#lmg_0.add_line(number=2,  name="Outlet %d" % 3,  computer=switch, voltage=220, description="Outlet %d" % 3)
#lmg_0.add_line(number=3,  name="Outlet %d" % 4,  computer=amd5,   voltage=220, description="Outlet %d" % 4)

lmg_1= LMG450Device(name="LMG450-1", url="/dev/ttyUSB1", max_frequency=20)
lmg_1.add_line(number=0,  name="Outlet %d" % 1,  computer=intel2, voltage=220, description="Outlet %d" % 1)
lmg_1.add_line(number=1,  name="Outlet %d" % 2,  computer=intel1, voltage=220, description="Outlet %d" % 2)
lmg_1.add_line(number=2,  name="Outlet %d" % 3,  computer=amd1,   voltage=220, description="Outlet %d" % 3)
lmg_1.add_line(number=3,  name="Outlet %d" % 4,  computer=amd2,   voltage=220, description="Outlet %d" % 4)

lmg_2= LMG450Device(name="LMG450-2", url="/dev/ttyUSB0", max_frequency=20)
lmg_2.add_line(number=0,  name="Outlet %d" % 1,  computer=intel4, voltage=220, description="Outlet %d" % 1)
lmg_2.add_line(number=1,  name="Outlet %d" % 2,  computer=intel3, voltage=220, description="Outlet %d" % 2)
lmg_2.add_line(number=2,  name="Outlet %d" % 3,  computer=amd3,   voltage=220, description="Outlet %d" % 3)
lmg_2.add_line(number=3,  name="Outlet %d" % 4,  computer=amd4,   voltage=220, description="Outlet %d" % 4)

### --- Arduino Device --- ####
#ard= ArduPowerDevice(name="ArduPowerDevice", computer=intel5, url="/dev/ttyACM0", max_frequency=100)
#ard.add_line(number= 0, name="24p-01c-3.3v",    voltage= 3.3, description="", slope=71.4943627331, offset=198.6262)
#ard.add_line(number= 1, name="HDD-SAT-5v",      voltage=   5, description="", slope=69.9332761719, offset=199.6384)
#ard.add_line(number= 2, name="24p-02c-3.3v",    voltage= 3.3, description="", slope=72.0016991302, offset=197.0501)
#ard.add_line(number= 3, name="HDD-SAT-12v",     voltage=  12, description="", slope=70.8100589449, offset=197.2023)
#ard.add_line(number= 4, name="24p-04c-5v",      voltage=   5, description="", slope=72.0078867850, offset=198.6462)
#ard.add_line(number= 5, name="08p-2nd-12v",     voltage=  12, description="", slope=69.6503691096, offset=190.5429)
#ard.add_line(number= 6, name="24p-06c-5v",      voltage=   5, description="", slope=71.2795878411, offset=198.7789)
#ard.add_line(number= 7, name="08p-1st-12v",     voltage=  12, description="", slope=65.9691784663, offset=195.5635)
#ard.add_line(number= 8, name="24p-23c-5v",      voltage=   5, description="", slope=71.8246793103, offset=191.7053)
#ard.add_line(number= 9, name="08p-1st-12v",     voltage=  12, description="", slope=67.0736180009, offset=194.3389)
#ard.add_line(number=10, name="24p-12,13c-3.3v", voltage= 3.3, description="", slope=71.0547210500, offset=194.4443)
#ard.add_line(number=11, name="24p-10c-12v",     voltage=  12, description="", slope=68.1271175427, offset=190.9717)
#ard.add_line(number=12, name="08p-2nd-12v",     voltage=  12, description="", slope=71.5951535529, offset=194.5920)
#ard.add_line(number=13, name="24p-11c-12v",     voltage=  12, description="", slope=67.8861848384, offset=194.9146)
#ard.add_line(number=14, name="24p-21c-5v",      voltage=   5, description="", slope=72.1452259359, offset=196.7969)
#ard.add_line(number=15, name="24p-22c-5v",      voltage=   5, description="", slope=70.4234197519, offset=191.5707)


### --- Arduino Device --- ####
ard= ArduPowerDevice(name="ArduPowerDevice", computer=intel5, url="/dev/ttyACM0", max_frequency=100)
ard.add_line(number= 0, name="24p-01c-3.3v",    voltage= 3.3, description="", slope=71.4943627331, offset=194.12)
ard.add_line(number= 1, name="HDD-SAT-5v",      voltage=   5, description="", slope=69.9332761719, offset=192.67)
ard.add_line(number= 2, name="24p-02c-3.3v",    voltage= 3.3, description="", slope=72.0016991302, offset=193.11)
ard.add_line(number= 3, name="HDD-SAT-12v",     voltage=  12, description="", slope=70.8100589449, offset=193.07)
ard.add_line(number= 4, name="24p-04c-5v",      voltage=   5, description="", slope=72.0078867850, offset=194.97)
ard.add_line(number= 5, name="08p-2nd-12v",     voltage=  12, description="", slope=69.6503691096, offset=186.98)
ard.add_line(number= 6, name="24p-06c-5v",      voltage=   5, description="", slope=71.2795878411, offset=195.57)
ard.add_line(number= 7, name="08p-1st-12v",     voltage=  12, description="", slope=65.9691784663, offset=192.37)
ard.add_line(number= 8, name="24p-23c-5v",      voltage=   5, description="", slope=71.8246793103, offset=188.22)
ard.add_line(number= 9, name="08p-1st-12v",     voltage=  12, description="", slope=67.0736180009, offset=191.60)
ard.add_line(number=10, name="24p-12,13c-3.3v", voltage= 3.3, description="", slope=71.0547210500, offset=191.25)
ard.add_line(number=11, name="24p-10c-12v",     voltage=  12, description="", slope=68.1271175427, offset=188.17)
ard.add_line(number=12, name="08p-2nd-12v",     voltage=  12, description="", slope=71.5951535529, offset=191.68)
ard.add_line(number=13, name="24p-11c-12v",     voltage=  12, description="", slope=67.8861848384, offset=192.63)
ard.add_line(number=14, name="24p-21c-5v",      voltage=   5, description="", slope=72.1452259359, offset=194.16)
ard.add_line(number=15, name="24p-22c-5v",      voltage=   5, description="", slope=70.4234197519, offset=189.42)

