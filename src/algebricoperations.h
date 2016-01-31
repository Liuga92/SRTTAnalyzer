
#ifndef SRTTANALYSIS_ALGEBRICOPERATIONS_H
#define SRTTANALYSIS_ALGEBRICOPERATIONS_H

#include <stddef.h>
#include <gsl/gsl_math.h>

/*
The function takes as argumenta pointer to an array representing a
flattened squared matrix (row1 concatened row2 concatened row3 ..) of doubles,
called "flat_squared_mat", whose number of rows (and columns) is "dimension".
The function computes the heigenvalues and heigenvectors of the matrix and
returns the eigenvalue nearest to 1 and his corresponding heigenvectors
in the array of doubles called "result" (whose memory was allocated by the user)
that has to be a size of equal to "dimension"
*/
double algop_get_best_eigen(double flat_squared_mat[],
			    size_t dimension, double result[]);


#endif
