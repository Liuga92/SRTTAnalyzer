
#include "convolutions.h"

#include <stddef.h>
#include <stdio.h>

void
convolutions_convolve (const double first[], size_t first_len,
		       const double second[], size_t second_len,
		       double convolved[])
{
  size_t n;

  for (n = 0; n < first_len + second_len - 1; n++)
    {
      size_t kmin, kmax, k;

      convolved[n] = 0;

      kmin = (n >= second_len - 1) ? n - (second_len - 1) : 0;
      kmax = (n < first_len - 1) ? n : first_len - 1;

      for (k = kmin; k <= kmax; k++)
	{
	  convolved[n] += first[k] * second[n - k];

	}

    }

}

int
convolutions_shrink_convolve (const double first[], size_t first_len,
			      const double second[], size_t second_len,
			      size_t delta_second, double convolved[])
{

  const double *shifted_first;
  double *shifted_convolved;
  size_t k;
  if (second_len < delta_second)
    return 0;

  shifted_first = &(first[delta_second]);
  shifted_convolved = &(convolved[delta_second]);

  for (k = 0; k < delta_second; k++)
    {
      convolved[k] = first[k];

    }

  convolutions_convolve (shifted_first, first_len - delta_second, second,
			 second_len, shifted_convolved);

  return 1;
}
