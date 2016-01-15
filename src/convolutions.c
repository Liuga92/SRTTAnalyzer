
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
convolutions_split_convolve (const double first[], size_t first_len,
			     const double second[], size_t second_len,
			     size_t delta_second, double convolved[])
{

  const double *shifted_first;
  double *shifted_convolved;
  int k;
  if (first_len < delta_second)
    {
      for (k = 0; k < first_len; k++)
	{
	  convolved[k] = first[k];
	}
      return 1;
    }

  shifted_first = &(first[delta_second + 1]);
  shifted_convolved = &(convolved[delta_second + 1]);
  convolutions_convolve (shifted_first, first_len - (delta_second + 1),
			 second, second_len, shifted_convolved);

  for (k = 0; k <= delta_second; k++)
    {
      convolved[k] = first[k];

    }


  return 1;
}

int
convolutions_convolve_shrink (const double first[], size_t first_len,
			      const double second[], size_t second_len,
			      size_t delta_second, double convolved[])
{
  int i;

  if (delta_second > first_len || delta_second > second_len)
    {
      return 0;
    }

  convolutions_convolve (first, first_len, second, second_len, convolved);

  for (i = 1; i <= delta_second; i++)
    {
      // printf("%lf\n", convolved[0]);
      convolved[0] += convolved[i];
    }
  for (i = 1; i < first_len + second_len; i++)
    {
      convolved[i] = convolved[i + delta_second];
    }

  return 1;


}
