
#ifndef SRTTANALYSIS_CONVOLUTIONS_H
#define SRTTANALYSIS_CONVOLUTIONS_H

#include <stddef.h>

void
convolutions_convolve (const double first[], size_t firstLen,
		       const double second[], size_t secondLen,
		       double convolved[]);

int
convolutions_shrink_convolve (const double first[], size_t firstLen,
			      const double second[], size_t secondLen,
			      size_t deltaSecond, double convolved[]);
#endif
