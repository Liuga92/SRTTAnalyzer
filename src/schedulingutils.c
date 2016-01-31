
#include "stochasticanalysis.h"

#include <stdlib.h>
#include <string.h>
#include "convolutions.h"
#include "algebricoperations.h"

/*rerurns the hyperperiod of a taskset*/
static unsigned int get_hyperperiod(stochastic_taskset * ts);
/*computes lowest common multiple*/
static unsigned int get_lcm(unsigned int a, unsigned int b);

/*computes the mean of a distribution*/
static double distr_mean(double *distribution, size_t distr_len);






/*
returns a task schedule object for the stochastic taskset "ts"
*/
tasks_schedule *new_task_schedule(stochastic_taskset * ts)
{
    tasks_schedule *t_sched;

    if (!ts)
	return NULL;

    t_sched = malloc(sizeof(tasks_schedule));

    if (!t_sched)
	return NULL;
    t_sched->ts = ts;
    t_sched->hyperperiod = get_hyperperiod(ts);

    {
	int i;
	unsigned int t_num = 0;

	for (i = 0; i < ts->tasks_num; i++) {
	    t_num += t_sched->hyperperiod / ts->task_list[i]->period;
	}
	t_sched->timeline_len = t_num;
    }

    t_sched->timeline.is_new = 1;
    get_max_idle_FP(t_sched, 0);

    return t_sched;
}

/*
frees the memory of the task schedule passed AND THE ASSOCIATED TASKSET
*/
void free_task_schedule(tasks_schedule * t_sched)
{
    if (!t_sched)
	return;

    free_stochastic_taskset(t_sched->ts);
    free(t_sched);
}

int is_stable(tasks_schedule * t_sched)
{
    double average_utilization;

    average_utilization = get_average_utilization(t_sched->ts);

    if (average_utilization <= 1)
	return 1;
    else
	return 0;
}


double get_average_utilization(stochastic_taskset * ts)
{
    int i;
    double avg_util = 0.0;
    for (i = 0; i < ts->tasks_num; i++) {
	stochastic_task *t;

	t = ts->task_list[i];
	avg_util += distr_mean(t->sd.dist, t->sd.d_len) / t->period;
    }
    return avg_util;
}


static unsigned int get_hyperperiod(stochastic_taskset * ts)
{
    unsigned int hyperperiod = 0;
    unsigned int i = 0;

    if (ts->tasks_num == 1) {
	return ts->task_list[0]->period;
    }

    for (i = 1; i < ts->tasks_num; i++) {

	if (i == 1) {
	    hyperperiod =
		get_lcm(ts->task_list[0]->period,
			ts->task_list[1]->period);
	} else {
	    hyperperiod = get_lcm(hyperperiod, ts->task_list[i]->period);
	}
    }

    return hyperperiod;

}

static unsigned int get_lcm(unsigned int a, unsigned int b)
{
    int tmp;

    tmp = (a > b) ? a : b;

    while ((tmp % a != 0) || (tmp % b != 0)) {
	tmp++;
    }

    return tmp;



}

/*
iterates over the hyperperiod, setting the timeline struct to the next activation
of the next process. returns 1 if there's another process that needs to
be scheduled in the hyperperiod, 0 otherwise.
*/
int next_task(tasks_schedule * t_sched)
{
    unsigned int current_time;
    unsigned int current_task;



    if (t_sched->timeline.is_new) {
	current_time = 0;
	current_task = 0;
	t_sched->timeline.is_new = 0;
    } else {
	if (t_sched->ts->tasks_num == t_sched->timeline.id + 1) {
	    current_time = t_sched->timeline.release_time + 1;
	    current_task = 0;
	} else {
	    current_time = t_sched->timeline.release_time;
	    current_task = t_sched->timeline.id + 1;
	}
    }

    {

	int go_ahead = 1;
	unsigned int i;
	unsigned int task_copy = current_task;
	for (i = current_time; go_ahead && i < t_sched->hyperperiod - 1;
	     i++) {
	    unsigned int j;
	    for (j = task_copy; go_ahead && j < t_sched->ts->tasks_num;
		 j++) {
		stochastic_task *t;
		t = t_sched->ts->task_list[j];

		if ((i % t->period) - t->activation_time == 0) {
		    go_ahead = 0;
		    current_task = j;
		    current_time = i;
		}
	    }

	    task_copy = 0;
	}

	if (go_ahead) {
	    t_sched->timeline.is_new = 1;
	    return 0;
	} else {
	    t_sched->timeline.id = current_task;
	    t_sched->timeline.release_time = current_time;
	    return 1;
	}


    }

}



static double distr_mean(double *distribution, size_t distr_len)
{
    int i;
    double mean = 0.0;

    for (i = 0; i < distr_len; i++) {
	mean += i * distribution[i];
    }

    return mean;

}

unsigned int
get_max_idle_FP(tasks_schedule * t_sched, unsigned int kth_task)
{
    unsigned int max_idle = 0;
    task_timeline *tl;

    tl = &t_sched->timeline;
    max_idle += t_sched->hyperperiod;


    /*computing W_min */
    {
	unsigned int W_min = 0;
	unsigned int prev_time, prev_duration;

	do {
	    next_task(t_sched);
	}
	while (has_priority_FP(t_sched->ts, kth_task, tl->id));

	prev_time = t_sched->timeline.release_time;
	prev_duration = min_index(&t_sched->ts->task_list[tl->id]->sd);
	W_min = prev_duration;

	while (next_task(t_sched)) {
	    if (!has_priority_FP(t_sched->ts, kth_task, tl->id)) {
		W_min = (W_min > tl->release_time - prev_time) ?
		    W_min - (tl->release_time - prev_time) : 0;

		prev_duration =
		    min_index(&t_sched->ts->task_list[tl->id]->sd);

		prev_time = t_sched->timeline.release_time;
		W_min += prev_duration;
	    }
	}
	W_min = (W_min > t_sched->hyperperiod - prev_time) ?
	    W_min - (t_sched->hyperperiod - prev_time) : 0;

	max_idle += W_min;
    }
    /*computing C_min */
    {
	unsigned int C_min = 0;
	while (next_task(t_sched)) {
	    if (!has_priority_FP(t_sched->ts, kth_task, tl->id)) {
		C_min += min_index(&t_sched->ts->task_list[tl->id]->sd);
	    }

	}
	max_idle -= C_min;
    }


    return max_idle;
}

int
has_priority_FP(stochastic_taskset * ts, unsigned int this,
		unsigned int over_this)
{
    if (ts->task_list[this]->deadline < ts->task_list[over_this]->deadline)
	return 1;
    else
	return 0;
}
