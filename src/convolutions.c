
#include "convolutions.h"

#include <stddef.h>
#include <stdio.h>

void
convolutions_convolve (const double first[], size_t firstLen,
		       const double second[], size_t secondLen,
		       double convolved[])
{
  size_t n;

  for (n = 0; n < firstLen + secondLen - 1; n++)
    {
      size_t kmin, kmax, k;

      convolved[n] = 0;

      kmin = (n >= secondLen - 1) ? n - (secondLen - 1) : 0;
      kmax = (n < firstLen - 1) ? n : firstLen - 1;

      for (k = kmin; k <= kmax; k++)
	{
	  convolved[n] += first[k] * second[n - k];

	}

    }

}

int
convolutions_shrink_convolve (const double first[], size_t firstLen,
			      const double second[], size_t secondLen,
			      size_t deltaSecond, double convolved[])
{
  const double *shiftedFirst;
  double *shiftedConvolved;
  size_t k;
  if (secondLen < deltaSecond)
    return 0;

  shiftedFirst = &(first[deltaSecond]);
  shiftedConvolved = &(convolved[deltaSecond]);

  for (k = 0; k < deltaSecond; k++)
    {
      convolved[k] = first[k];

    }

  convolutions_convolve (shiftedFirst, firstLen - deltaSecond, second,
			 secondLen, shiftedConvolved);

  return 1;
}
