
#include "algebricoperations.h"

#include <math.h>
#include <float.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>


double
algop_get_best_eigen (double flat_squared_mat[],
		      size_t dimension, double result[])
{


  gsl_matrix_view m;
  gsl_vector_complex *eval;
  gsl_matrix_complex *evec;
  gsl_eigen_nonsymmv_workspace *w;

  m = gsl_matrix_view_array (flat_squared_mat, dimension, dimension);

  eval = gsl_vector_complex_alloc (dimension);
  evec = gsl_matrix_complex_alloc (dimension, dimension);

  w = gsl_eigen_nonsymmv_alloc (dimension);

  gsl_eigen_nonsymmv (&m.matrix, eval, evec, w);

  gsl_eigen_nonsymmv_free (w);


  {
    int i, best;
    double best_diff = DBL_MAX;
    double best_value;
    gsl_vector_complex_view evec_i;
    gsl_complex z;

    for (i = 0; i < dimension; i++)
      {
	double diff;
	gsl_complex eval_i;

	eval_i = gsl_vector_complex_get (eval, i);
	diff = fabs (1.0 - GSL_REAL (eval_i));
	if (diff < best_diff)
	  {
	    best = i;
	    best_diff = diff;
	    best_value = GSL_REAL (eval_i);
	  }

      }


    evec_i = gsl_matrix_complex_column (evec, best);

    for (i = 0; i < dimension; i++)
      {
	z = gsl_vector_complex_get (&evec_i.vector, i);
	result[i] = GSL_REAL (z);
      }

    gsl_vector_complex_free (eval);
    gsl_matrix_complex_free (evec);


    return best_value;
  }

}
