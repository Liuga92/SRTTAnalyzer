CFLAGS = -Wall -g
LDLIBS = -lgsl -lgslcblas -lm

sources = algebricoperations.c convolutions.c stochasticanalysis.c \
            stochastictasks.c ioutils.c schedulingutils.c


rtsanalysis: rtsanalysis.c $(sources:.c=.o)





clean:
	rm -f *.d
	rm -f *.o
	rm -f *.c~
	rm -f *.h~
	rm -f rtsanalysis
	rm -f test


include $(sources:.c=.d)

%.d: %.c #as suggested by the gnu make doc https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
