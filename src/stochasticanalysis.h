
#ifndef SRTTANALYSIS_STOCHASTICANALYSIS_H
#define SRTTANALYSIS_STOCHASTICANALYSIS_H

#include "stochastictasks.h"

typedef struct
{
  unsigned int id;
  unsigned int release_time;
} task_timeline;


typedef struct
{
  unsigned int hyperperiod;
  task_timeline *timeline;
  stochastic_taskset ts;
} tasks_schedule;

typedef struct
{
  size_t len;
  double *matrix;
  unsigned int id;
  tasks_schedule sched;
} backlog_matrix;

/*
returns a task schedule object for the stochastic taskset "ts"
*/
tasks_schedule new_task_schedule (stochastic_taskset ts);
/*
computes the backlog matrix for the kth task. The size parameter is the
dimension of the matrix. The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix get_backlog_matrix_of_size (tasks_schedule t_sched,
					   unsigned int kth_task,
					   size_t size);


/*
Get the minimum dimension for the backlog matrix for kth process.
*/
size_t get_minimum_matrix_size (tasks_schedule t_sched,
				unsigned int kth_task);
/*
get an array containing the response time distribution for the given
backlog matrix
*/
double *get_response_time_of (backlog_matrix b_mat);

#endif
