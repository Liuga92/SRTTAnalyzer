
#ifndef SRTTANALYSIS_STOCHASTICANALYSIS_H
#define SRTTANALYSIS_STOCHASTICANALYSIS_H

#include "stochastictasks.h"
#include "schedulingutils.h"
#include <stdlib.h>



/*backlog matrix for a certain task*/
typedef struct {
    size_t dimension;		/*len of matrix */
    double *matrix;
    unsigned int id;		/*id of the task */
    tasks_schedule *sched;	/*linked task schedule */
} backlog_matrix;


/*
computes the backlog matrix for the kth task. The size parameter is the
dimension (number of rows/columns) of the matrix.
The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix *get_backlog_matrix_of_size(tasks_schedule * t_sched,
					   unsigned int kth_task,
					   size_t size);

/*
frees the memory taken by the matrix NOT THE ONE OF THE ASSOICIATED TASK SHEDULE
*/
void free_backlog_matrix(backlog_matrix * bk_mat);
/*
Get the minimum dimension (number of row/columns)
for the backlog matrix for kth process.
*/
size_t get_minimum_matrix_size(tasks_schedule * t_sched,
			       unsigned int kth_task);
/*
get a new stochastic_distribution containing the stationary distribution for the given
backlog matrix (that is associated with a specific task)
*/
stochastic_distribution *get_stationary_distribution(backlog_matrix *
						     b_mat);
/*
get an new stochastic_distribution of the mean response for the given
backlog matrix (that is associated with a specific task). The distribution
is correct UP TO THE DEADLINE. The remaining probability is collapsed in
the time DEADLINE + 1
*/
stochastic_distribution *get_mean_response_time_of(backlog_matrix * b_mat);


#endif
