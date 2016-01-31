
#include "stochastictasks.h"
#include "convolutions.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/*check if the distribution sums (with small error) to 1*/
static int check_distr(const double *distribution, size_t distr_len);
/*increases the size of the array of tasks of the given taskset*/
static int increase_taskset(stochastic_taskset * ts);


/*value of the last wrong distribution submation*/
static double wrong_dist_submation = 0.0;
/* if different from 0 prints and returns an error if the
distribution does not sum to 1*/
static int show_distribution_errors = 1;



stochastic_distribution *split_convolve_SD(stochastic_distribution * first,
					   stochastic_distribution *
					   second, size_t delta_second)
{

    stochastic_distribution *r;

    r = new_stochastic_distribution(first->d_len + second->d_len);
    if (!r)
	return NULL;

    convolutions_split_convolve(first->dist, first->d_len, second->dist,
				second->d_len, delta_second, r->dist);

    return r;

}


/*
convolves and shrink two stochastic dstributions and creates a new one as result.
Same logic of "convolutions_convolve_shrink" in source "convolutions.h"
*/
stochastic_distribution *convolve_shrink_SD(stochastic_distribution *
					    first,
					    stochastic_distribution *
					    second, size_t delta_second)
{
    stochastic_distribution *r;

    r = new_stochastic_distribution(first->d_len + second->d_len);
    if (!r) {
	return NULL;
    }
    /*if something goes wrong abort */
    if (0 ==
	convolutions_convolve_shrink(first->dist, first->d_len,
				     second->dist, second->d_len,
				     delta_second, r->dist)) {
	free_stochastic_distribution(r);
	return NULL;
    }

    return r;

}

unsigned int max_index(stochastic_distribution * d)
{
    int i;

    for (i = d->d_len - 1; i >= 0; i--) {
	if (d->dist[i] > 0) {
	    return i;
	}
    }

    return 0;
}


unsigned int min_index(stochastic_distribution * d)
{
    unsigned int i;

    for (i = 0; i < d->d_len; i++) {
	if (d->dist[i] > 0)
	    return i;
    }

    return 0;
}

void minimize_stochastic_distribution(stochastic_distribution * s_dist)
{
    unsigned int max_i;

    max_i = max_index(s_dist);
    s_dist->d_len = max_i + 1;
    s_dist->dist = realloc(s_dist->dist, s_dist->d_len * sizeof(double));
}

stochastic_task_view
new_stochastic_task_view(double *distribution,
			 size_t d_len,
			 unsigned int deadline,
			 unsigned int period, unsigned int activation_time)
{

    stochastic_task_view task;



    task.deadline = deadline;
    task.period = period;
    task.activation_time = activation_time;
    task.sd.d_len = d_len;
    task.sd.dist = distribution;

    return task;
}

stochastic_distribution *new_stochastic_distribution(size_t size)
{
    stochastic_distribution *t;

    t = malloc(sizeof(stochastic_distribution));
    if (!t)
	return NULL;
    t->d_len = size;
    t->dist = calloc(size, sizeof(double));
    if (!t->dist) {
	free(t);
	return NULL;
    }
    memset(t->dist, 0, size * sizeof(double));

    return t;
}

stochastic_distribution *copy_stochastic_distribution(const
						      stochastic_distribution
						      * s_dist)
{
    stochastic_distribution *d;
    d = new_stochastic_distribution(s_dist->d_len);
    if (!d)
	return NULL;
    memcpy(d->dist, s_dist->dist, s_dist->d_len * sizeof(double));

    return d;


}

stochastic_distribution *add_stochastic_distribution(const
						     stochastic_distribution
						     * first,
						     const
						     stochastic_distribution
						     * second)
{

    stochastic_distribution *sum;
    size_t max_size;

    max_size =
	(first->d_len > second->d_len) ? first->d_len : second->d_len;

    sum = new_stochastic_distribution(max_size);
    if (!sum)
	return NULL;

    {
	unsigned int i;
	for (i = 0; i < first->d_len; i++) {
	    sum->dist[i] += first->dist[i];
	}
	for (i = 0; i < second->d_len; i++) {
	    sum->dist[i] += second->dist[i];
	}
    }

    return sum;
}

void normalize_stochastic_distribution(stochastic_distribution * d)
{
    double submation = 0.0;
    unsigned int i;
    for (i = 0; i < d->d_len; i++) {
	submation += d->dist[i];
    }
    for (i = 0; i < d->d_len; i++) {
	d->dist[i] /= submation;
    }
}

void free_stochastic_distribution(stochastic_distribution * s_dist)
{
    if (!s_dist)
	return;

    free(s_dist->dist);
    free(s_dist);
}

/*creates a new stochastic task in the heap*/
stochastic_task *new_stochastic_task(const double *distribution,
				     size_t distr_len,
				     unsigned int deadline,
				     unsigned int period,
				     unsigned int activation_time)
{
    stochastic_task *task;



    /*if the distribution is wrong and the flag is set returns an error */
    if (!check_distr(distribution, distr_len) && show_distribution_errors) {
	fprintf(stderr, "task's distribution with deadline %i,\
	  period %i, activation time %i does not sum to one. the submation is %lf\n", deadline, period, activation_time, wrong_dist_submation);
	return NULL;
    }



    task = malloc(sizeof(stochastic_task));

    if (!task) {
	return NULL;
    }

    task->deadline = deadline;
    task->period = period;
    task->activation_time = activation_time;
    task->sd.d_len = distr_len;
    task->sd.dist = calloc(distr_len, sizeof(double));

    if (!task->sd.dist) {
	return NULL;
    }
    {
	int i;
	for (i = 0; i < distr_len; i++) {
	    task->sd.dist[i] = distribution[i];
	}
    }
    return task;
}


stochastic_taskset *new_stochastic_taskset(size_t task_num)
{
    stochastic_taskset *taskset;

    taskset = malloc(sizeof(stochastic_taskset));

    if (!taskset) {
	return NULL;
    }

    taskset->tasks_num = 0;
    taskset->task_list_len = task_num;
    taskset->task_list = calloc(task_num, sizeof(stochastic_task *));

    if (!taskset->task_list) {
	return NULL;
    }

    return taskset;

}

void free_stochastic_task(stochastic_task * task)
{
    if (!task) {
	return;
    }
    free(task->sd.dist);
    free(task);
}




void free_stochastic_taskset(stochastic_taskset * ts)
{
    int i;

    if (!ts) {
	return;
    }
    for (i = 0; i < ts->tasks_num; i++) {
	free_stochastic_task(ts->task_list[i]);
    }
    free(ts->task_list);
    free(ts);

}



int add_task(stochastic_taskset * ts, stochastic_task_view task)
{
    if (ts->task_list_len == ts->tasks_num) {
	if (!increase_taskset(ts)) {

	    return 0;
	}
    }

    ts->task_list[ts->tasks_num] = new_stochastic_task(task.sd.dist,
						       task.sd.d_len,
						       task.deadline,
						       task.period,
						       task.activation_time);

    if (!ts->task_list[ts->tasks_num]) {
	return 0;
    }
    ts->tasks_num += 1;

    return 1;
}


void set_disribution_errors(int flag)
{
    if (flag) {
	show_distribution_errors = 1;
    } else {
	show_distribution_errors = 0;
    }
}






static int increase_taskset(stochastic_taskset * ts)
{
    ts->task_list = realloc(ts->task_list, ts->task_list_len * 2);

    if (ts->task_list) {
	ts->task_list_len *= 2;
	return 1;
    }

    else {
	return 0;
    }

}



static int check_distr(const double *distribution, size_t distr_len)
{
    double submation = 0.0;
    int i;

    for (i = 0; i < distr_len; i++) {
	submation += distribution[i];
    }

    if (fabs(submation - 1) > 10e-6) {
	wrong_dist_submation = submation;
	return 0;
    }
    return 1;
}
