CC=gcc
#CC=icc

CC_OPTIONS=-Wall -fPIC

OBJ= pm_set_server.o \
     pm_start_counter.o \
     pm_stop_counter.o \
     pm_get_counter_data.o \
     pm_finalize_counter.o \
     pm_continue_counter.o \
     pm_create_counter.o \
     pm_get_device_info.o \
     pm_get_devices.o \
     pm_set_lines.o \
     pm_print_data_text.o \
     pm_print_data_csv.o \
     pm_print_data_paraver.o \
     pm_print_data_stdout.o \
     pm_print_data_paraver_cstates.o \
     pm_print_data_paraver_pstates.o \
     pm_get_and_print_data_paraver.o \
     pm_calculate_energy.o \
     pm_fprint_counter_info.o 
#     pm_print_data_otf.o \

LIB=libpmlib.so
LIB=pmlib.a 
PLUGIN_LIB=libPmlibPlugin.so

VT_INSTALLDIR=/home/dolz/Install/VampirTrace_gcc_install
# OTF_INC=-I/home/dolz/otf_install/include/open-trace-format/

INSTALL=/home/dolz/pmlib

all: $(LIB) $(PLUGIN_LIB)

$(LIB):$(OBJ)
	ar -cr $(LIB) $(OTF_LIB) $(OBJ)
	ranlib $(LIB)

$(PLUGIN_LIB):$(LIB) vt_pmlib_plugin.c
	$(CC) -c -fPIC vt_pmlib_plugin.c -o libPmlibPlugin.o -I$(VT_INSTALLDIR)/include -I$(INSTALL) `pkg-config --cflags --libs glib-2.0`
	$(CC) -shared -Wl,-soname,$(PLUGIN_LIB) -o $(PLUGIN_LIB) libPmlibPlugin.o $(LIB) `pkg-config --libs glib-2.0`

.c.o:
	$(CC) $(CC_OPTIONS) $(OTF_INC) -D_LARGEFILE64_SOURCE -c $<

install: all
	install -m 644 pmlib.a $(INSTALL)/lib/
	install -m 644 line.h pmlib.h $(INSTALL)/include/

clean:
	rm -rf $(OBJ) $(LIB) libPmlibPlugin.o libPmlibPlugin.so
