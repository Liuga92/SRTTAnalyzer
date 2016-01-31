
#ifndef SRTTANALYSIS_CONVOLUTIONS_H
#define SRTTANALYSIS_CONVOLUTIONS_H

#include <stddef.h>

/*
The function does the convolution of two  array "first" and "second",
with size "first_len" and "second_len". The result is stored in the array
"convolved" whose size has to be at least "first_len" +"second_len".
*/
void
convolutions_convolve(const double first[], size_t first_len,
		      const double second[], size_t second_len,
		      double convolved[]);

/*
Same as before, except the fact that the operation performed is the
splitting and convolution described into the paper .
The parameter "delta_second"
the positive shift that has to be applied to second
 distribution respect to the first.
*/

int
convolutions_split_convolve(const double first[], size_t first_len,
			    const double second[], size_t second_len,
			    size_t delta_second, double convolved[]);


/*
Same as before, except the fact that the operation performed is the
convolution and shrinking described into the paper .
The parameter "delta_second"
 is the shift that the second array is subject during the shrinking.
*/

int
convolutions_convolve_shrink(const double first[], size_t first_len,
			     const double second[], size_t second_len,
			     size_t delta_second, double convolved[]);

/*
shrink a distribution of an amount equal to "delta"
*/
int convolutions_shrink(double dist[], size_t size, size_t delta);

#endif
