
#include "algebricoperations.h"

#include <math.h>
#include <float.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>


double
algop_get_best_eigen (double flat_squared_mat[],
		      size_t dimension, double result[])
{
  int i, best_index, stop_iteration;
  double best_diff, best_eigenvalue;
  gsl_vector *eval;
  gsl_matrix *evec;
  gsl_eigen_symmv_workspace *wkspace;
  gsl_vector_view best_eigenvector;
  gsl_matrix_view m;

  m = gsl_matrix_view_array (flat_squared_mat, dimension, dimension);

  eval = gsl_vector_alloc (dimension);
  evec = gsl_matrix_alloc (dimension, dimension);

  wkspace = gsl_eigen_symmv_alloc (dimension);

  gsl_eigen_symmv (&m.matrix, eval, evec, wkspace);

  gsl_eigen_symmv_free (wkspace);

  gsl_eigen_symmv_sort (eval, evec, GSL_EIGEN_SORT_ABS_ASC);


  best_diff = DBL_MAX;
  stop_iteration = 0;
  for (i = 0; i < dimension && !stop_iteration; i++)
    {
      double diff, eigenvalue;

      eigenvalue = gsl_vector_get (eval, i);
      diff = fabs (1.0 - eigenvalue);

      if (diff < best_diff)
	{
	  best_diff = diff;
	  best_index = i;
	  best_eigenvalue = eigenvalue;
	}
      else if (eigenvalue - 1.0 > 0)
	{
	  stop_iteration = 1;
	}
    }
  best_eigenvector = gsl_matrix_column (evec, best_index);

  for (i = 0; i < dimension; i++)
    {
      result[i] = gsl_vector_get (&best_eigenvector.vector, i);
    }

  gsl_vector_free (eval);
  gsl_matrix_free (evec);

  return best_eigenvalue;
}
