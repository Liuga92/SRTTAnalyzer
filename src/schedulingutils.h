
#ifndef SRTTANALYSIS_SCHEDULINGUTILS_H
#define SRTTANALYSIS_SCHEDULINGUTILS_H

#include "stochastictasks.h"
#include <stdlib.h>

/*struct that contains data to iterato over the hyperperiod on the taskset*/
typedef struct {
    int is_new;			/*is a new iteration */
    unsigned int id;		/*id of the next task to activate */
    int release_time;		/*releasing time WRT the hyperperiod */
} task_timeline;

/*
struct representing a schedule of a taskset
*/
typedef struct {
    unsigned int hyperperiod;	/*hyperperiod */
    unsigned int timeline_len;	/*number of tasks activated during a hyperperiod */
    task_timeline timeline;
    stochastic_taskset *ts;
} tasks_schedule;




/*
returns a task schedule object for the stochastic taskset "ts"
*/
tasks_schedule *new_task_schedule(stochastic_taskset * ts);

/*
frees the memory of the task schedule passed AND THE ASSOCIATED TASKSET
*/
void free_task_schedule(tasks_schedule * t_sched);

/*
Check if the system is stable (if the average system utilization is < 1)
*/
int is_stable(tasks_schedule * t_sched);
/*
return average utilization for the given task set
*/
double get_average_utilization(stochastic_taskset * ts);

/*iterates over the tasks of a task_schedule over the hyperperiod,
setting the timeline struct inside t_sched to the id of the
next job that has to start and the relative (respect to the
hyperperiod) starting time*/
int next_task(tasks_schedule * t_sched);

/*
check in a taskset with fixed priorities
task "this" has priority respect to the task "over_this".
*/
int has_priority_FP(stochastic_taskset * ts, unsigned int this,
		    unsigned int over_this);
/*
gets the maximum idle time << called "r" inside the paper >>
of a taskset (inside a task schedule)
with fixed priorities, respect to the kth task of the taskset.
*/
unsigned int get_max_idle_FP(tasks_schedule * t_sched,
			     unsigned int kth_task);



#endif
