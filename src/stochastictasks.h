
#ifndef SRTTANALYSIS_STOCHASTICTASK_H
#define SRTTANALYSIS_STOCHASTICTASK_H

#include <stddef.h>
#include <stdio.h>

/*
Struct representing a single task.
*/
typedef struct
{
  size_t d_len;
  double *distribution;
  unsigned int deadline;
  unsigned int period;
  unsigned int activation_time;
  unsigned int identifier;
  unsigned int priority;
} stochastic_task;

/*

*/
typedef struct
{
  size_t d_len;
  double *distribution;
  unsigned int deadline;
  unsigned int period;
  unsigned int activation_time;
} stochastic_task_view;


/*
Struct representing a taskset.
*/
typedef struct
{
  size_t tasks_num;
  size_t task_list_len;
  stochastic_task **task_list;
} stochastic_taskset;


/*
creates a new stochastic task in the stack from an array of doubles
"distribution" with length "distr_len",
that represent the distribution of the time required by the task.
"deadline", "period" and "activation_time" are the deadline of the task,
its period and its activation time.
*/
stochastic_task_view new_stochastic_task_view (double *distribution,
					       size_t distr_len,
					       unsigned int deadline,
					       unsigned int period,
					       unsigned int activation_time);


// stochastic_task* new_stochastic_task (const double *distribution,
//                                   size_t distr_len,
//                                   unsigned int deadline,
//                                   unsigned int period,
//                                   unsigned int activation_time);
/*
creates a new task set with space for a number of tasks equal to "task_num"
*/
stochastic_taskset *new_stochastic_taskset (size_t task_num);
/*
frees the memory of a stochastic task
*/
void free_stochastic_task (stochastic_task * task);
/*
frees the memory of a stochastic taskset AND  THE MEMORY OF ITS TASKS.
For each of them the "free_stochastic_task" function has to be called
*/
void free_stochastic_taskset (stochastic_taskset * ts);
/*
adds a task to a taskset
*/
int add_task (stochastic_taskset * ts, stochastic_task_view task);



#endif
