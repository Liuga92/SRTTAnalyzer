
#include "stochasticanalysis.h"

#include <stdlib.h>
#include <string.h>
#include "convolutions.h"
#include "algebricoperations.h"
#include "stochastictasks.h"
#include "ioutils.h"


/*
computes the response time for a job (the one currently saved in the timeline struct
inside the schedluer inside the backlog matrix) starting from the jab backlog passed;
*/
static stochastic_distribution *get_response_time_of_job(backlog_matrix *
							 b_mat,
							 stochastic_distribution
							 * job_back);



/*
computes the baklog distribution starting from "starting_dist" (if different
from null) for task with id "task_id" (inside the taskset),
until time "until time"( <= hyperperiod)
*/
static stochastic_distribution *compute_backlog_until(tasks_schedule *
						      t_sched,
						      unsigned int task_id,
						      unsigned int
						      until_time,
						      const
						      stochastic_distribution
						      * starting_dist);


/*
computes the backlog matrix for the kth task. The size parameter is the
dimension of the matrix. The minimum is the one returned by the function
"get_minimum_matrix_size"
*/
backlog_matrix *get_backlog_matrix_of_size(tasks_schedule * t_sched,
					   unsigned int kth_task,
					   size_t size)
{

    backlog_matrix *b_mat;
    /*if the size is not enough, abort */
    {
	size_t min_size;

	min_size = get_minimum_matrix_size(t_sched, kth_task);
	if (size < min_size) {
	}
    }
    /*allocating and initializing the matrix */
    b_mat = malloc(sizeof(backlog_matrix));
    if (!b_mat)
	return NULL;
    b_mat->matrix = calloc(size * size, sizeof(double));
    if (!b_mat->matrix) {
	free(b_mat);
	return NULL;
    }
    memset(b_mat->matrix, 0, size * size * sizeof(double));
    b_mat->dimension = size;
    b_mat->id = kth_task;
    b_mat->sched = t_sched;

    {
	unsigned int r, m_r;
	stochastic_distribution *b_r;
	stochastic_distribution *r_d;
	/*getting r */
	r = get_max_idle_FP(b_mat->sched, b_mat->id);
	//distribution representing the a backlog of size r
	r_d = new_stochastic_distribution(r + 1);
	if (!r_d) {
	    free_backlog_matrix(b_mat);
	    return NULL;
	}
	r_d->dist[r] = 1.0;

	// reinitialize the timeline
	b_mat->sched->timeline.is_new = 1;
	// computing the column b_r
	b_r = compute_backlog_until(b_mat->sched, b_mat->id,
				    b_mat->sched->hyperperiod, r_d);

	if (!b_r) {
	    free_backlog_matrix(b_mat);
	    free_stochastic_distribution(r_d);
	    return NULL;
	}

	m_r = max_index(b_r);


	{
	    /*filling the matrix from to r up the last */
	    unsigned int i;
	    /*for every column  >= r fill the rows with b_r */
	    for (i = 0; i < b_mat->dimension - r; i++) {
		unsigned int j;
		for (j = 0; j <= m_r; j++) {
		    /*if the is inside the truncated matrix */
		    if (j + i < b_mat->dimension) {
			/*fill the cell of the matrix */
			b_mat->matrix[(i + r) + (i + j) * size] =
			    b_r->dist[j];
		    }
		}
	    }
	}

	/*fill the matrix from row 0 to r-1 */
	free_stochastic_distribution(b_r);
	{
	    int i, j;
	    stochastic_distribution *b_i;
	    for (i = 0; i < r; i++) {
		/*distribution representing the a backlog of size i */
		memset(r_d->dist, 0, r_d->d_len * sizeof(double));
		r_d->dist[i] = 1;
		b_mat->sched->timeline.is_new = 1;
		/*computing the column b_i */
		b_i = compute_backlog_until(b_mat->sched, b_mat->id,
					    b_mat->sched->hyperperiod,
					    r_d);

		if (!b_r) {
		    free_backlog_matrix(b_mat);
		    free_stochastic_distribution(r_d);
		    return NULL;
		}
		/*filling cells of the matrix */
		for (j = 0; j < b_i->d_len && j <= m_r; j++) {
		    b_mat->matrix[b_mat->dimension * j + i] = b_i->dist[j];
		}
		free_stochastic_distribution(b_i);
	    }
	}

	free_stochastic_distribution(r_d);
    }

    return b_mat;

}




/*
frees the memory taken by the matrix NOT THE ONE OF THE ASSOCIATED TASK SHEDULE
*/
void free_backlog_matrix(backlog_matrix * bk_mat)
{
    free(bk_mat->matrix);
    free(bk_mat);

}

/*
Get the minimum dimension for the backlog matrix for kth process.
*/
size_t
get_minimum_matrix_size(tasks_schedule * t_sched, unsigned int kth_task)
{
    unsigned int r, mr;
    stochastic_distribution *sd;
    stochastic_distribution *bkr_sd;

    /* getting maximum idle time */
    r = get_max_idle_FP(t_sched, kth_task);

    bkr_sd = new_stochastic_distribution(r + 1);
    if (!bkr_sd)
	return 0;

    bkr_sd->dist[r] = 1.0;
    /*getting b_r (colunm r of the backlog matrix) */
    sd = compute_backlog_until(t_sched, kth_task, t_sched->hyperperiod,
			       bkr_sd);

    if (!sd) {
	free_stochastic_distribution(bkr_sd);
	return 0;
    }

    /*computing m_r (last nonzero cell fo the column b_r) */
    mr = max_index(sd);


    free_stochastic_distribution(sd);
    free_stochastic_distribution(bkr_sd);

    /*returning the bigger */
    if (r > mr)
	return r + 1;
    else
	return mr + 1;

}


stochastic_distribution *get_stationary_distribution(backlog_matrix *
						     b_mat)
{

    stochastic_distribution *b_stationary;

    b_stationary = new_stochastic_distribution(b_mat->dimension);
    if (!b_stationary)
	return NULL;

    algop_get_best_eigen(b_mat->matrix, b_mat->dimension,
			 b_stationary->dist);

    {
	int i;
	double subm = 0.0;
	for (i = 0; i < b_stationary->d_len; i++) {
	    subm += b_stationary->dist[i];
	}
	for (i = 0; i < b_stationary->d_len; i++) {
	    b_stationary->dist[i] /= subm;
	}
    }
    return b_stationary;

}

/*
get a new stochastic_distribution containing the stationary distribution for the given
backlog matrix (that is associated with a specific task)
*/
stochastic_distribution *get_mean_response_time_of(backlog_matrix * b_mat)
{

    stochastic_distribution *b_stat, *job_back, *task_resp, *job_resp,
	*temporary;
    task_timeline saved_timeline;
    task_timeline *tl;

    tl = &b_mat->sched->timeline;
    task_resp = new_stochastic_distribution(1);
    if (!task_resp)
	return NULL;
    b_stat = get_stationary_distribution(b_mat);
    if (!b_stat) {
	free_stochastic_distribution(task_resp);
	return NULL;

    }
    // initializing the copy of the timeline (to avoid proble at value 0)
    saved_timeline.release_time = -1;
    //initializing the timeline
    tl->is_new = 1;

    while (next_task(b_mat->sched)) {
	/*if a new instance of the task starts (the repvious is saved in
	   saved_timeline) */
	if (tl->id == b_mat->id
	    && saved_timeline.release_time < tl->release_time) {
	    /*save this instance */
	    saved_timeline = *tl;
	    /*reinitialize the timeline */
	    tl->is_new = 1;
	    /*compute the backlog until this instance of the task */
	    job_back =
		compute_backlog_until(b_mat->sched, b_mat->id,
				      saved_timeline.release_time, b_stat);
	    if (!job_back) {
		free_stochastic_distribution(task_resp);
		free_stochastic_distribution(b_stat);
		return NULL;
	    }
	    /*restore the current timeline */
	    *tl = saved_timeline;
	    /*compute the response time for this job */
	    job_resp = get_response_time_of_job(b_mat, job_back);
	    /*restorea again the current timeline */
	    *tl = saved_timeline;
	    if (!job_resp) {
		free_stochastic_distribution(task_resp);
		free_stochastic_distribution(b_stat);
		free_stochastic_distribution(job_back);
		return NULL;
	    }

	    temporary = add_stochastic_distribution(task_resp, job_resp);

	    free_stochastic_distribution(task_resp);
	    if (!temporary) {
		free_stochastic_distribution(b_stat);
		free_stochastic_distribution(job_back);
		return NULL;
	    }
	    task_resp = temporary;
	}
    }
    free_stochastic_distribution(b_stat);
    free_stochastic_distribution(job_back);

    normalize_stochastic_distribution(task_resp);
    /*if the activation tiem is != from 0, shifts the distribution to
       the left to compensate */
    shift_stochastic_distribution(task_resp,
				  b_mat->sched->ts->task_list[b_mat->id]->
				  activation_time);

    /*accumulate the probability over the deadline into deadline+1 */
    {
	unsigned int i, deadline;
	deadline = b_mat->sched->ts->task_list[b_mat->id]->deadline;
	for (i = deadline + 2; i < task_resp->d_len; i++) {
	    task_resp->dist[deadline + 1] += task_resp->dist[i];
	    task_resp->dist[i] = 0.0;
	}
    }
    return task_resp;
}

/*
computes the response time for a job (the one currently saved in the timeline struct
inside the schedluer inside the backlog matrix) starting from the jab backlog passed;
*/
static stochastic_distribution *get_response_time_of_job(backlog_matrix *
							 b_mat,
							 stochastic_distribution
							 * job_back)
{

    unsigned int abs_deadl, abs_release;
    stochastic_taskset *ts;
    stochastic_distribution *resp_time, *temporary;
    task_timeline *tl;

    /*pointers used to make less verbore the code below */
    ts = b_mat->sched->ts;
    tl = &b_mat->sched->timeline;

    abs_release = tl->release_time;
    abs_deadl = ts->task_list[b_mat->id]->deadline + abs_release;

    resp_time =
	convolve_shrink_SD(job_back, &ts->task_list[tl->id]->sd, 0);

    if (!resp_time)
	return NULL;

    while (next_task(b_mat->sched) && tl->release_time < abs_deadl) {
	if (has_priority_FP(ts, tl->id, b_mat->id)) {

	    temporary =
		split_convolve_SD(resp_time, &ts->task_list[tl->id]->sd,
				  tl->release_time - abs_release);
	    free_stochastic_distribution(resp_time);

	    if (!temporary)
		return NULL;

	    resp_time = temporary;
	}
    }
    return resp_time;
}




static stochastic_distribution *compute_backlog_until(tasks_schedule *
						      t_sched,
						      unsigned int task_id,
						      unsigned int
						      until_time,
						      const
						      stochastic_distribution
						      * starting_dist)
{

    stochastic_distribution *d;
    stochastic_distribution *start;
    size_t previous_time;
    task_timeline *tl;
    stochastic_taskset *ts;

    /*pointers used to make less verbore the code below */
    ts = t_sched->ts;		//the taskset
    tl = &t_sched->timeline;	//the timeline

    /*checking if there's a starting distribution */
    if (starting_dist) {
	start = copy_stochastic_distribution(starting_dist);
	previous_time = 0;
    }
    /*if not, loading the first task as starting distribution */
    else {
	do {
	    next_task(t_sched);

	}
	while (has_priority_FP(ts, task_id, tl->id));
	start = copy_stochastic_distribution(&ts->task_list[tl->id]->sd);
	previous_time = tl->release_time;

    }
    /*while there's task and the time is not expired, convolve shrink the next task */
    while (next_task(t_sched) && tl->release_time <= until_time) {


	if (tl->release_time == until_time) {
	}
	/* if the current task in the timeline has priority */
	if (has_priority_FP(ts, tl->id, task_id)
	    /*or if the two tasks has the same priority and the release time
	       of the task in the timeline is less than the limit */
	    || (!has_priority_FP(ts, tl->id, task_id)
		&& !has_priority_FP(ts, task_id, tl->id)
		&& tl->release_time < until_time)) {

	    d = convolve_shrink_SD(start, &ts->task_list[tl->id]->sd,
				   tl->release_time - previous_time);

	    if (!d) {
		return NULL;
	    }
	    previous_time = tl->release_time;
	    free_stochastic_distribution(start);
	    start = d;

	}
    }
    /*shrink the distribution of the remenaining delta */
    convolutions_shrink(start->dist, start->d_len,
			until_time - previous_time);

    return start;
}
