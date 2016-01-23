
#include "stochastictasks.h"

#include <math.h>
#include <stdlib.h>


static int check_distr (const double *distribution, size_t distr_len);
static int increase_taskset (stochastic_taskset * ts);

stochastic_task_view
new_stochastic_task_view (double *distribution,
			  size_t d_len,
			  unsigned int deadline,
			  unsigned int period, unsigned int activation_time)
{

  stochastic_task_view task;


  if (!check_distr (distribution, d_len))
    {
      fprintf (stderr, "task's distribution with deadline %i,\
period %i, activation time %i does not sum to one.\n", deadline, period, activation_time);
    }

  task.deadline = deadline;
  task.period = period;
  task.activation_time = activation_time;
  task.d_len = d_len;
  task.distribution = distribution;

  return task;
}


stochastic_task *
new_stochastic_task (const double *distribution,
		     size_t distr_len,
		     unsigned int deadline,
		     unsigned int period, unsigned int activation_time)
{
  stochastic_task *task;




  if (!check_distr (distribution, distr_len))
    {
      fprintf (stderr, "task's distribution with deadline %i,\
period %i, activation time %i does not sum to one.\n", deadline, period, activation_time);
      return NULL;
    }



  task = malloc (sizeof (stochastic_task));

  if (!task)
    {
      return NULL;
    }

  task->deadline = deadline;
  task->period = period;
  task->activation_time = activation_time;
  task->d_len = distr_len;
  task->distribution = calloc (distr_len, sizeof (double));

  if (!task->distribution)
    {
      return NULL;
    }
  {
    int i;
    for (i = 0; i < distr_len; i++)
      {
	task->distribution[i] = distribution[i];
      }
  }
  return task;
}


stochastic_taskset *
new_stochastic_taskset (size_t task_num)
{
  stochastic_taskset *taskset;

  taskset = malloc (sizeof (stochastic_taskset));

  if (!taskset)
    {
      return NULL;
    }

  taskset->tasks_num = 0;
  taskset->task_list_len = task_num;
  taskset->task_list = calloc (task_num, sizeof (stochastic_task *));

  if (!taskset->task_list)
    {
      return NULL;
    }

  return taskset;

}

void
free_stochastic_task (stochastic_task * task)
{
  if (!task)
    {
      return;
    }
  free (task->distribution);
  free (task);
}




void
free_stochastic_taskset (stochastic_taskset * ts)
{
  int i;

  if (!ts)
    {
      return;
    }
  for (i = 0; i < ts->tasks_num; i++)
    {
      free_stochastic_task (ts->task_list[i]);
    }
  free (ts->task_list);
  free (ts);

}



int
add_task (stochastic_taskset * ts, stochastic_task_view task)
{
  if (ts->task_list_len == ts->tasks_num)
    {
      if (!increase_taskset (ts))
	{

	  return 0;
	}
    }

  ts->task_list[ts->tasks_num] = new_stochastic_task (task.distribution,
						      task.d_len,
						      task.deadline,
						      task.period,
						      task.activation_time);

  if (!ts->task_list[ts->tasks_num])
    {
      return 0;
    }
  ts->tasks_num += 1;

  return 1;
}









static int
increase_taskset (stochastic_taskset * ts)
{
  ts->task_list = realloc (ts->task_list, ts->task_list_len * 2);

  if (ts->task_list)
    {
      ts->task_list_len *= 2;
      return 1;
    }

  else
    {
      return 0;
    }

}



static int
check_distr (const double *distribution, size_t distr_len)
{
  double submation = 0.0;
  int i;

  for (i = 0; i < distr_len; i++)
    {
      submation += distribution[i];
    }

  if (fabs (submation - 1) > 10e-6)
    {
      return 0;
    }
  return 1;
}
