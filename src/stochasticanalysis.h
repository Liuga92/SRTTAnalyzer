
#ifndef SRTTANALYSIS_STOCHASTICANALYSIS_H
#define SRTTANALYSIS_STOCHASTICANALYSIS_H

#include "stochastictasks.h"
#include <stdlib.h>

/*struct that contains data to iterato over the hyperperiod on the taskset*/
typedef struct
{
  int is_new; /*is a new iteration*/
  unsigned int id; /*id of the next task to activate*/
  unsigned int release_time; /*releasing time WRT the hyperperiod*/
} task_timeline;

/*
struct representing a schedule of a taskset
*/
typedef struct
{
  unsigned int hyperperiod; /*hyperperiod*/
  unsigned int timeline_len; /*number of tasks activated during a hyperperiod*/
  task_timeline timeline;
  stochastic_taskset *ts;
} tasks_schedule;

/*backlog matrix for a certain task*/
typedef struct
{
  size_t len; /*len of matrix*/
  double *matrix;
  unsigned int id; /*id of the task*/
  tasks_schedule * sched; /*linked task schedule*/
} backlog_matrix;



/*
returns a task schedule object for the stochastic taskset "ts"
*/
tasks_schedule *new_task_schedule (stochastic_taskset * ts);

/*
frees the memory of the task schedule passed AND THE ASSOCIATED TASKSET
*/
void free_task_schedule (tasks_schedule * t_sched);

/*
Check if the system is stable (if the average system utilization is < 1)
*/
int is_stable (tasks_schedule * t_sched);
/*
computes the backlog matrix for the kth task. The size parameter is the
dimension (number of rows/columns) of the matrix.
The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix *get_backlog_matrix_of_size (tasks_schedule * t_sched,
					    unsigned int kth_task,
					    size_t size);

/*
frees the memory taken by the matrix NOT THE ONE OF THE ASSOICIATED TASK SHEDULE
*/
void free_backlog_matrix (backlog_matrix * bk_mat);
/*
Get the minimum dimension (number of row/columns)
for the backlog matrix for kth process.
*/
size_t get_minimum_matrix_size (tasks_schedule * t_sched,
				unsigned int kth_task);
/*
get an array containing the response time distribution for the given
backlog matrix (that is associated with a specific task). The distribution
is correct UP TO THE DEADLINE. The remaining probability is collapsed in
the time DEADLINE + 1.
*/
stochastic_distribution *get_response_time_of (backlog_matrix  *  b_mat);



#endif
