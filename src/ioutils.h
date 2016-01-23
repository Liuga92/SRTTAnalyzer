#ifndef SRTTANALYSIS_IOUTILS_H
#define SRTTANALYSIS_IOUTILS_H

#include "stochastictasks.h"

/*
load taskset from a File. File format TBD.
*/
stochastic_taskset *load_stochastic_taskset (FILE * source, char delimiter[]);

#endif
