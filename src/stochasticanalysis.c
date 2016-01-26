
#include "stochasticanalysis.h"

#include <stdlib.h>
#include "convolutions.h"

/*rerurns the hyperperiod of a taskset*/
static unsigned int get_hyperperiod (stochastic_taskset * ts);
/*computes lowest common multiple*/
static unsigned int get_lcm (unsigned int a, unsigned int b);
/*iterates over the tasks of a task_schedule over the hyperperiod,
setting the timeline struct inside t_sched to the id of the
next job that has to start and the relative (respect to the
hyperperiod) starting time*/
static int next_task (tasks_schedule * t_sched);
/*computes the mean of a distribution*/
static double distr_mean (double *distribution, size_t distr_len);
/*
gets the maximum idle time << called "r" inside the paper >>
of a taskset (inside a task schedule)
with fixed priorities, respect to the kth task of the taskset.
*/
static unsigned int get_max_idle_FP (tasks_schedule * t_sched,
				     unsigned int kth_task);
/*
get the first index whose value is different from 0 inside a distributon
*/
static unsigned int min_index (double *distribution, size_t d_len);
/*
check in a taskset with fixed priorities
task "this" has priority respect to the task "over_this".
*/
static int has_priority_FP (stochastic_taskset * ts, unsigned int this,
			 unsigned int over_this);
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
returns a task schedule object for the stochastic taskset "ts"
*/
tasks_schedule *
new_task_schedule (stochastic_taskset * ts)
{
  tasks_schedule *t_sched;

  if (!ts)
    return NULL;

  t_sched = malloc (sizeof (tasks_schedule));

  if (!t_sched)
    return NULL;
  t_sched->ts = ts;
  t_sched->hyperperiod = get_hyperperiod (ts);

  {
    int i;
    unsigned int t_num = 0;

    for (i = 0; i < ts->tasks_num; i++)
      {
	t_num += t_sched->hyperperiod / ts->task_list[i]->period;
      }
    t_sched->timeline_len = t_num;
  }

  t_sched->timeline.is_new = 1;
  get_max_idle_FP (t_sched, 0);

  return t_sched;
}

/*
frees the memory of the task schedule passed AND THE ASSOCIATED TASKSET
*/
void
free_task_schedule (tasks_schedule * t_sched)
{
  if (!t_sched)
    return;

  free_stochastic_taskset (t_sched->ts);
  free (t_sched);
}

int
is_stable (tasks_schedule * t_sched)
{
  int i;
  double average_utilization = 0.0;
  for (i = 0; i < t_sched->ts->tasks_num; i++)
    {
      average_utilization +=
	distr_mean (t_sched->ts->task_list[i]->sd.dist,
		    t_sched->ts->task_list[i]->sd.d_len) /
	t_sched->ts->task_list[i]->period;
    }
  printf ("average utilization %lf\n", average_utilization);
  if (average_utilization <= 1)
    return 1;
  else
    return 0;
}


/*
computes the backlog matrix for the kth task. The size parameter is the
dimension of the matrix. The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix *get_backlog_matrix_of_size (tasks_schedule * t_sched,
					    unsigned int kth_task,
					    size_t size);

/*
frees the memory taken by the matrix NOT THE ONE OF THE ASSOCIATED TASK SHEDULE
*/
void free_backlog_matrix (backlog_matrix * bk_mat);
/*
Get the minimum dimension for the backlog matrix for kth process.
*/
size_t get_minimum_matrix_size (tasks_schedule * t_sched,
				unsigned int kth_task)
{
	unsigned int r,mr;
	stochastic_distribution * sd;
	stochastic_distribution * bkr_sd;

	/* getting maximum idle time*/
	r = get_max_idle_FP(t_sched,kth_task);

	bkr_sd = new_stochastic_distribution(r +1);
	if(! bkr_sd) return 0;

	bkr_sd->dist[r] = 1.0;
	/*getting b_r (colunm r of the backlog matrix)*/
	sd = compute_backlog_until(t_sched,kth_task,t_sched->hyperperiod,bkr_sd);

	if(! sd){
		free_stochastic_distribution(bkr_sd);
		return 0;
	}

	/*computing m_r (last nonzero cell fo the column b_r)*/
	{
		int i,found;
		found = 0;
		for(i=sd->d_len-1;i>=0 && found == 0;i--){
			if(sd->dist[i] >0) {
				found = 1;
				mr = i;
				}
		}
	}

	free_stochastic_distribution(sd);
	free_stochastic_distribution(bkr_sd);

	/*returning the bigger*/
	if (r > mr) return r + 1;
	else return mr + 1;

}
/*
get an array containing the response time distribution for the given
backlog matrix (that is associated with a specific task). The distribution
is correct UP TO THE DEADLINE. The remaining probability is collapsed in
the time DEADLINE + 1.
*/
stochastic_distribution *get_response_time_of (backlog_matrix * b_mat);



static unsigned int
get_hyperperiod (stochastic_taskset * ts)
{
  unsigned int hyperperiod = 0;
  unsigned int i = 0;

  if (ts->tasks_num == 1)
    {
      return ts->task_list[0]->period;
    }

  for (i = 1; i < ts->tasks_num; i++)
    {

      if (i == 1)
	{
	  hyperperiod =
	    get_lcm (ts->task_list[0]->period, ts->task_list[1]->period);
	}
      else
	{
	  hyperperiod = get_lcm (hyperperiod, ts->task_list[i]->period);
	}
    }

  return hyperperiod;

}

static unsigned int
get_lcm (unsigned int a, unsigned int b)
{
  int tmp;

  tmp = (a > b) ? a : b;

  while ((tmp % a != 0) || (tmp % b != 0))
    {
      tmp++;
    }

  return tmp;



}

/*
iterates over the hyperperiod, setting the timeline struct to the next activation
of the next process. returns 1 if there's another process that needs to
be scheduled in the hyperperiod, 0 otherwise.
*/
static int
next_task (tasks_schedule * t_sched)
{
  unsigned int current_time;
  unsigned int current_task;



  if (t_sched->timeline.is_new)
    {
      current_time = 0;
      current_task = 0;
      t_sched->timeline.is_new = 0;
    }
  else
    {
      if (t_sched->ts->tasks_num == t_sched->timeline.id + 1)
	{
	  current_time = t_sched->timeline.release_time + 1;
	  current_task = 0;
	}
      else
	{
	  current_time = t_sched->timeline.release_time;
	  current_task = t_sched->timeline.id + 1;
	}
    }

  {

    int go_ahead = 1;
    unsigned int i;
    unsigned int task_copy = current_task;
    for (i = current_time; go_ahead && i < t_sched->hyperperiod; i++)
      {
	unsigned int j;
	for (j = task_copy; go_ahead && j < t_sched->ts->tasks_num; j++)
	  {
	    stochastic_task *t;
	    t = t_sched->ts->task_list[j];

	    if ((i % t->period) - t->activation_time == 0)
	      {
		go_ahead = 0;
		current_task = j;
		current_time = i;
	      }
	  }

	task_copy = 0;
      }

    if (go_ahead)
      {
	t_sched->timeline.is_new = 1;
	return 0;
      }
    else
      {
	t_sched->timeline.id = current_task;
	t_sched->timeline.release_time = current_time;
	return 1;
      }


  }

}



static double
distr_mean (double *distribution, size_t distr_len)
{
  int i;
  double mean = 0.0;

  for (i = 0; i < distr_len; i++)
    {
      mean += i * distribution[i];
    }

  return mean;

}

static unsigned int
get_max_idle_FP (tasks_schedule * t_sched, unsigned int kth_task)
{
  unsigned int max_idle = 0;
  task_timeline *tl;

  tl = &t_sched->timeline;
  max_idle += t_sched->hyperperiod;

  if (kth_task >= t_sched->ts->tasks_num)
    {
      printf ("error! computing max idle of task %i but task len is %zu\n",
	      kth_task, t_sched->ts->tasks_num);
    }

	/*computing W_min*/
  {
    unsigned int W_min = 0;
    unsigned int prev_time, prev_duration;

    do
      {
	next_task (t_sched);
      }
    while (has_priority_FP (t_sched->ts, kth_task, tl->id));

    prev_time = t_sched->timeline.release_time;
    prev_duration =
      min_index (t_sched->ts->task_list[tl->id]->sd.dist,
		 t_sched->ts->task_list[tl->id]->sd.d_len);
    W_min = prev_duration;

    while (next_task (t_sched))
      {
	if (!has_priority_FP (t_sched->ts, kth_task, tl->id))
	  {
	    W_min = (W_min > tl->release_time - prev_time) ?
	      W_min - (tl->release_time - prev_time) : 0;

	    prev_duration =
	      min_index (t_sched->ts->task_list[tl->id]->sd.dist,
			 t_sched->ts->task_list[tl->id]->sd.d_len);

	    prev_time = t_sched->timeline.release_time;
	    W_min += prev_duration;
	  }
      }
    W_min = (W_min > t_sched->hyperperiod - prev_time) ?
      W_min - (t_sched->hyperperiod - prev_time) : 0;

    max_idle += W_min;
  }
	/*computing C_min*/
  {
    unsigned int C_min = 0;
    while (next_task (t_sched))
      {
	if (!has_priority_FP (t_sched->ts, kth_task, tl->id))
	  {
	    C_min += min_index (t_sched->ts->task_list[tl->id]->sd.dist,
				t_sched->ts->task_list[tl->id]->sd.d_len);
	  }

      }
    max_idle -= C_min;
  }


  return max_idle;
}

static int
has_priority_FP (stochastic_taskset * ts, unsigned int this,
	      unsigned int over_this)
{
  if (ts->task_list[this]->deadline < ts->task_list[over_this]->deadline)
    return 1;
  else
    return 0;
}

static unsigned int
min_index (double *distribution, size_t d_len)
{
  unsigned int i;

  for (i = 0; i < d_len; i++)
    {
      if (distribution[i] > 0)
	return i;
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

	/*checking if there's a starting distribution*/
  if (starting_dist)
    {
      start = copy_stochastic_distribution (starting_dist);
      previous_time = 0;
    }
	/*if not, loading the first task as starting distribution*/
  else
    {
      do
	{
	  next_task (t_sched);

	}
      while (has_priority_FP (t_sched->ts, task_id, t_sched->timeline.id));
      start =
	copy_stochastic_distribution (&t_sched->ts->
				      task_list[t_sched->timeline.id]->sd);
      previous_time = t_sched->timeline.release_time;

    }
	/*while there's task and the time is not expired, convolve shrink the next task*/
  while (next_task (t_sched) && (t_sched->timeline.release_time < until_time))
    {
      if (!has_priority_FP (t_sched->ts, task_id, t_sched->timeline.id))
	{
	  d = convolve_shrink_SD (start,
				  &t_sched->ts->task_list[t_sched->timeline.id]->sd,
				  t_sched->timeline.release_time - previous_time);

	  if (!d){
      return NULL;
      }

	  previous_time = t_sched->timeline.release_time;
	  free_stochastic_distribution (start);
	  start = d;

	}
    }
	/*shrink to cover the gap between the last released time and the target time*/
	convolutions_shrink(d->dist,d->d_len,until_time - t_sched->timeline.release_time);

  return d;
}
