
#ifndef SRTTANALYSIS_STOCHASTICTASK_H
#define SRTTANALYSIS_STOCHASTICTASK_H

#include <stddef.h>
#include <stdio.h>


/*
a stochastic distribution
*/
typedef struct {
    size_t d_len;		/*its len */
    double *dist;		/*the distribution */
} stochastic_distribution;

/*
Struct representing a single task.
*/
typedef struct {
    stochastic_distribution sd;	/*its distribution */
    unsigned int deadline;
    unsigned int period;
    unsigned int activation_time;
} stochastic_task;

/*
object representing a task in the stack
*/
typedef struct {
    stochastic_distribution sd;	/*its distribution */
    unsigned int deadline;
    unsigned int period;
    unsigned int activation_time;
} stochastic_task_view;


/*
Struct representing a taskset.
*/
typedef struct {
    size_t tasks_num;		/*num of task */
    size_t task_list_len;	/*len of task list (can be different from tasks_num) */
    stochastic_task **task_list;	/*array of pointers to tasks */
} stochastic_taskset;




/*
split and convolves two stochastic dstributions and creates a new one as result
Same logic of "convolutions_split_convolve"  in source "convolutions.h"
*/
stochastic_distribution *split_convolve_SD(stochastic_distribution * first,
					   stochastic_distribution *
					   second, size_t delta_second);


/*
convolves and shrink two stochastic dstributions and creates a new one as result.
Same logic of "convolutions_convolve_shrink" in source "convolutions.h"
*/
stochastic_distribution *convolve_shrink_SD(stochastic_distribution *
					    first,
					    stochastic_distribution *
					    second, size_t delta_second);
/*
creates a new stochastic distribution whose distribution is of  of size "size"
and initialized to 0;
*/
stochastic_distribution *new_stochastic_distribution(size_t size);

/*
returns a copy of the stocastic distribution given as argument
*/
stochastic_distribution *copy_stochastic_distribution(const
						      stochastic_distribution
						      * s_dist);

/*
shifts the distribution positively  of a value equal to delta.
*/
void shift_stochastic_distribution(stochastic_distribution * d,
				   unsigned int delta);


/*
frees a stochastic distribution
*/
void free_stochastic_distribution(stochastic_distribution * s_dist);


void minimize_stochastic_distribution(stochastic_distribution * s_dist);

/*
returns a new distribution that is the summations of the twos in argument
*/
stochastic_distribution *add_stochastic_distribution(const
						     stochastic_distribution
						     * first,
						     const
						     stochastic_distribution
						     * second);

/*
normalize the distribution
(modifies the distribution such that the sumbmation gives 1)
*/
void normalize_stochastic_distribution(stochastic_distribution * d);

/*
get the maximum index whose element is > 0
*/
unsigned int max_index(stochastic_distribution * d);

/*
get the minimum index whose element is > 0
*/
unsigned int min_index(stochastic_distribution * d);

/*
creates a new stochastic task in the stack from an array of doubles
"distribution" with length "distr_len",
that represent the distribution of the time required by the task.
"deadline", "period" and "activation_time" are the deadline of the task,
its period and its activation time.
*/
stochastic_task_view new_stochastic_task_view(double *distribution,
					      size_t distr_len,
					      unsigned int deadline,
					      unsigned int period,
					      unsigned int
					      activation_time);



/*
creates a new task set with space for a number of tasks equal to "task_num"
*/
stochastic_taskset *new_stochastic_taskset(size_t task_num);
/*
frees the memory of a stochastic task
*/
void free_stochastic_task(stochastic_task * task);
/*
frees the memory of a stochastic taskset AND  THE MEMORY OF ITS TASKS.
For each of them the "free_stochastic_task" function has to be called
*/
void free_stochastic_taskset(stochastic_taskset * ts);
/*
adds a task to a taskset
*/
int add_task(stochastic_taskset * ts, stochastic_task_view task);
/*
activates/deactivates distribution'e errors (if distribution's submation
is different from a value near 1). "flag" must be 0 if you want do shutdown
this  type of errors or a value different from 0 if you want to activate it.
Default : the errors are active.
*/

void set_disribution_errors(int flag);

#endif
