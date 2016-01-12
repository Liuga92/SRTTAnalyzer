CFLAGS = -Wall -g

sources = algebricoperations.c convolutions.c stochasticanalysis.c \
            stochastictasks.c rtsanalysis.c


rtsanalysis:

clean:
	rm -f *.d
	rm -f *.o
	rm -f rtsanalysis

include $(sources:.c=.d)


%.d: %.c #as suggested by the gnu make doc https://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
