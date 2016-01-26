#ifndef SRTTANALYSIS_IOUTILS_H
#define SRTTANALYSIS_IOUTILS_H

#include "stochastictasks.h"

/*
load taskset from a File. File firmat:
deadline,period,activation_time, ... distribution ...
(a task for every line, no blank lines, comment lines starting with '#')
the delimiter for every element inside a row is "delimiter"
*/
stochastic_taskset *load_stochastic_taskset (FILE * source, char delimiter[]);
/*
prints a stochastic distribution over a file
*/
int fprintf_stochastic_distribution(FILE* destination,stochastic_distribution * sd);

#endif
