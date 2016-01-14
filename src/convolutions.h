
#ifndef SRTTANALYSIS_CONVOLUTIONS_H
#define SRTTANALYSIS_CONVOLUTIONS_H

#include <stddef.h>

/*
The function does the convolution of two  array "first" and "second",
with size "first_len" and "second_len". The result is stored in the array
"convolved" whose size has to be at least "first_len" +"second_len".
*/
void
convolutions_convolve (const double first[], size_t first_len,
		       const double second[], size_t second_len,
		       double convolved[]);

int
/*
Same as before, except the fact that the operation performed is the
shrinking and convolution described into the paper. The parameter "delta_second"
is the shift that the second array is subject during the shrinking. 
*/


convolutions_shrink_convolve (const double first[], size_t first_len,
			      const double second[], size_t second_len,
			      size_t delta_second, double convolved[]);
#endif
