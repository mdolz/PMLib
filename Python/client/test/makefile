CC=icc
CC=gcc

LIBS  = ../pmlib.a -I..
CFLAGS = -Wall 

EXE = example0.x    \
      example1.x    \
      example2.x    \
      example3.x    \
      example4.x    \
      pm_measure

all: $(EXE)

%.x: %.c ../pmlib.a
	${CC} $< $(CFLAGS) $(LIBS) -o $@

pm_measure: pm_measure.c ../pmlib.a
	${CC} $< $(CFLAGS) $(LIBS) -lm -o $@
#	${CC} $< $(CFLAGS) $(LIBS) -L/home/dolz/otf_install/lib/ -lopen-trace-format -lm -o $@

clean:
	rm -rf *.x pm_measure
