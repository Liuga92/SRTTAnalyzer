
#include "stochasticanalysis.h"

#include <stdlib.h>
#include <string.h>
#include "convolutions.h"
#include "algebricoperations.h"
#include "ioutils.h"



/*
get the last index whose value is different from 0 inside a distributon
*/
static unsigned int max_index (double *distribution, size_t d_len);

/*
computes the baklog distribution starting from "starting_dist" (if different
from null) for task with id "task_id" (inside the taskset),
until time "until time"( <= hyperperiod)
*/
static stochastic_distribution *compute_backlog_until (tasks_schedule *
						       t_sched,
						       unsigned int task_id,
						       unsigned int
						       until_time,
						       const
						       stochastic_distribution
						       * starting_dist);


/*
computes the backlog matrix for the kth task. The size parameter is the
dimension of the matrix. The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix *
get_backlog_matrix_of_size (tasks_schedule * t_sched,
			    unsigned int kth_task, size_t size)
{

  backlog_matrix *b_mat;
  /*if the size is not enough, abort */
  {
    size_t min_size;

    min_size = get_minimum_matrix_size (t_sched, kth_task);
    if (size < min_size)
      {
	printf ("size too small, minimum size %zu\n", min_size);
      }
  }
  /*allocating and initializing the matrix */
  b_mat = malloc (sizeof (backlog_matrix));
  if (!b_mat)
    return NULL;
  b_mat->matrix = calloc (size * size, sizeof (double));
  if (!b_mat->matrix)
    {
      free (b_mat);
      return NULL;
    }
  memset (b_mat->matrix, 0, size * size * sizeof (double));

  b_mat->dimension = size;
  b_mat->id = kth_task;
  b_mat->sched = t_sched;

  {
    unsigned int r, m_r;
    stochastic_distribution *b_r;
    stochastic_distribution *r_d;
    /*getting r */
    r = get_max_idle_FP (b_mat->sched, b_mat->id);
    r_d = new_stochastic_distribution (r + 1);
    if (!r_d)
      {
	free_backlog_matrix (b_mat);
	return NULL;
      }
    r_d->dist[r] = 1.0;
    b_r = compute_backlog_until (b_mat->sched, b_mat->id,
				 b_mat->sched->hyperperiod, r_d);
    if (!b_r)
      {
	free_backlog_matrix (b_mat);
	free_stochastic_distribution (r_d);
	return NULL;
      }

    m_r = max_index (b_r->dist, b_r->d_len);

    {
      /*filling the matrix from to r up the last */
      unsigned int i;
      /*for every column  >= r fill the rows with b_r */
      for (i = 0; i < b_mat->dimension - r; i++)
	{
	  unsigned int j;
	  for (j = 0; j <= m_r; j++)
	    {
	      /*if the is inside the truncated matrix */
	      if (j + i < b_mat->dimension)
		{

		  b_mat->matrix[(i + r) + (i + j) * size] = b_r->dist[j];
		}
	      /*if not, accumulate teh probability at the highest index */
	      else
		{
		  b_mat->matrix[(size - 1) * size + i + r] += b_r->dist[j];
		}
	    }
	}
    }
    /*fill the matrix from row 0 to r-1 */
    free_stochastic_distribution (b_r);
    {
      int i, j;
      stochastic_distribution *b_i;
      for (i = 0; i < r; i++)
	{
	  memset (r_d->dist, 0, r_d->d_len * sizeof (double));
	  r_d->dist[i] = 1;

	  b_i = compute_backlog_until (b_mat->sched, b_mat->id,
				       b_mat->sched->hyperperiod, r_d);

	  if (!b_r)
	    {
	      free_backlog_matrix (b_mat);
	      free_stochastic_distribution (r_d);
	      return NULL;
	    }

	  for (j = 0; j < b_i->d_len && j <= m_r; j++)
	    {
	      b_mat->matrix[b_mat->dimension * j + i] = b_i->dist[j];
	    }

	  free_stochastic_distribution (b_i);

	}
    }

    free_stochastic_distribution (r_d);
  }

  return b_mat;

}

/*
frees the memory taken by the matrix NOT THE ONE OF THE ASSOCIATED TASK SHEDULE
*/
void
free_backlog_matrix (backlog_matrix * bk_mat)
{
  free (bk_mat->matrix);
  free (bk_mat);

}

/*
Get the minimum dimension for the backlog matrix for kth process.
*/
size_t
get_minimum_matrix_size (tasks_schedule * t_sched, unsigned int kth_task)
{
  unsigned int r, mr;
  stochastic_distribution *sd;
  stochastic_distribution *bkr_sd;

  /* getting maximum idle time */
  r = get_max_idle_FP (t_sched, kth_task);

  bkr_sd = new_stochastic_distribution (r + 1);
  if (!bkr_sd)
    return 0;

  bkr_sd->dist[r] = 1.0;
  /*getting b_r (colunm r of the backlog matrix) */
  sd =
    compute_backlog_until (t_sched, kth_task, t_sched->hyperperiod, bkr_sd);

  if (!sd)
    {
      free_stochastic_distribution (bkr_sd);
      return 0;
    }

  /*computing m_r (last nonzero cell fo the column b_r) */
  {
    int i, found;
    found = 0;
    for (i = sd->d_len - 1; i >= 0 && found == 0; i--)
      {
	if (sd->dist[i] > 10e-6)
	  {
	    found = 1;
	    mr = i;
	  }
      }
  }

  free_stochastic_distribution (sd);
  free_stochastic_distribution (bkr_sd);

  /*returning the bigger */
  if (r > mr)
    return r + 1;
  else
    return mr + 1;

}

/*
get an array containing the response time distribution for the given
backlog matrix (that is associated with a specific task). The distribution
is correct UP TO THE DEADLINE. The remaining probability is collapsed in
the time DEADLINE + 1.
*/
stochastic_distribution *
get_stationary_distribution (backlog_matrix * b_mat)
{

  stochastic_distribution *b_stationary;

  b_stationary = new_stochastic_distribution (b_mat->dimension);
  if (!b_stationary)
    return NULL;

  algop_get_best_eigen (b_mat->matrix, b_mat->dimension, b_stationary->dist);

  {
    int i;
    double subm = 0.0;
    for (i = 0; i < b_stationary->d_len; i++)
      {
	subm += b_stationary->dist[i];
      }
    for (i = 0; i < b_stationary->d_len; i++)
      {
	b_stationary->dist[i] /= subm;
      }
  }
  return b_stationary;

}

static unsigned int
max_index (double *distribution, size_t d_len)
{
  int i;

  for (i = d_len - 1; i >= 0; i--)
    {
      if (distribution[i] > 0)
	{
	  return i;
	}
    }

  return 0;
}


static stochastic_distribution *
compute_backlog_until (tasks_schedule *
		       t_sched, unsigned int task_id, unsigned int until_time,
		       const stochastic_distribution * starting_dist)
{
  stochastic_distribution *d;
  stochastic_distribution *start;
  size_t previous_time;

  /*checking if there's a starting distribution */
  if (starting_dist)
    {
      start = copy_stochastic_distribution (starting_dist);
      previous_time = 0;
    }
  /*if not, loading the first task as starting distribution */
  else
    {
      do
	{
	  next_task (t_sched);

	}
      while (has_priority_FP (t_sched->ts, task_id, t_sched->timeline.id));
      start =
	copy_stochastic_distribution (&t_sched->
				      ts->task_list[t_sched->timeline.id]->
				      sd);
      previous_time = t_sched->timeline.release_time;

    }
  /*while there's task and the time is not expired, convolve shrink the next task */
  while (next_task (t_sched) && (t_sched->timeline.release_time < until_time))
    {
      if (!has_priority_FP (t_sched->ts, task_id, t_sched->timeline.id))
	{
	  d = convolve_shrink_SD (start,
				  &t_sched->ts->task_list[t_sched->timeline.
							  id]->sd,
				  t_sched->timeline.release_time -
				  previous_time);

	  if (!d)
	    {
	      return NULL;
	    }

	  previous_time = t_sched->timeline.release_time;
	  free_stochastic_distribution (start);
	  start = d;

	}
    }
  /*shrink to cover the gap between the last released time and the target time */
  convolutions_shrink (d->dist, d->d_len,
		       until_time - t_sched->timeline.release_time);

  return d;
}
