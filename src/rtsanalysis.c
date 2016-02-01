#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "ioutils.h"
#include "stochastictasks.h"
#include "schedulingutils.h"
#include "stochasticanalysis.h"

int execute(int argc, char **argv);

int main(int argc, char **argv)
{
    return execute(argc, argv);
}

int execute(int argc, char **argv)
{
    int size_flag = 0;
    int query_flag = 0;
    int show_matrix_flag = 0;
    int task;
    int size = -1;
    int c;

    while ((c = getopt(argc, argv, "hqbs::t:")) != -1) {
	switch (c) {

	case 'q':
	    query_flag = 1;
	    break;
	case 's':{
		size_flag = 1;
		size = atoi(optarg);
	    };
	    break;
	case 'b':
	    show_matrix_flag = 1;
	    break;
	case 'h':{
		printf
		    ("utility for the computation of the rt distribution."
		     " The task set is read from the standard input\n");
        printf("WARNING the tasks must be ordered by the value of"\
        " the first column. The -t options is mandatory \n");
		printf
		    ("use -s <size> to set the size of the backlog matrix\n");
		printf
		    ("use -t <task num> to decide wich task to evaluate \n");
		printf
		    ("use -q to know the minimum size of the back matrices "
		     "for the tasks\n");
		return 0;
	    };
	    break;
	case 't':{
		task = atoi(optarg);
	    };
	    break;
	case '?':{
		if (optopt == 's' || optopt == 't') {
		    fprintf(stderr, "flag -%c requires an argument \n",
			    optopt);
		    return 1;
		} else if (isprint(optopt)) {
		    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
		    return 1;
		} else {
		    fprintf(stderr, "Unknown option character `\\x%x'.\n",
			    optopt);
		    return 1;
		}
	    };
	    break;
	}
    }

    {
	stochastic_taskset *ts;
	tasks_schedule *s;
	backlog_matrix *b_mat;
	stochastic_distribution *r_time;

	ts = load_stochastic_taskset(stdin, ",");
	if (!ts) {
	    fprintf(stderr, "it seems that the taskset is not well"
		    "formatted\n");
	    return 1;
	}
	s = new_task_schedule(ts);
	if (!s) {
	    fprintf(stderr, "error\n");
	    free_stochastic_taskset(ts);
	    return 1;
	}


	if (ts->tasks_num < task || task == 0) {
	    fprintf(stderr, "the taskset size is %zu, you asked for task"
		    " %i\n", ts->tasks_num, task);
	    return 1;
	}
	if (query_flag) {
	    unsigned int m;
	    m = get_minimum_matrix_size(s, task - 1);
	    printf("minimum matrix's size for task %i: %i\n", task - 1, m);
	    return 0;
	}
	if (size_flag) {
	    unsigned int m;
	    m = get_minimum_matrix_size(s, task - 1);
	    if (size < m) {
		fprintf(stderr,
			"error!minimum matrix's size for task %i: %i\n",
			task - 1, m);
		return 1;
	    }
	} else if (!size_flag) {
	    size = get_minimum_matrix_size(s, task - 1);
	}

	if (!is_stable(s)) {
	    fprintf(stderr, "warning, the taskset is not stable,"
		    " avg utilization: %lf\n",
		    get_average_utilization(ts));
	}

	b_mat = get_backlog_matrix_of_size(s, task - 1, size);
	if (!b_mat) {
	    if (!s) {
		fprintf(stderr, "error\n");
		free_task_schedule(s);
		return 1;
	    }
	}

	if (show_matrix_flag) {
	    printf("matrix's ");
	    fprintf_backlog_matrix(stdout, b_mat);
	    printf("\n");
	}

	r_time = get_mean_response_time_of(b_mat);
	if (!r_time) {
	    fprintf(stderr, "error\n");
	    return 1;

	}
	minimize_stochastic_distribution(r_time);
	printf("response time for task %i:\n", task);
	fprintf_stochastic_distribution(stdout, r_time);

    free_backlog_matrix(b_mat);
    free_task_schedule(s);
    }

    return 0;

}
