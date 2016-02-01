
#include "ioutils.h"

#include <stdlib.h>
#include <string.h>

/*gets the number of cells*/
static unsigned int get_cells(const char *string, const char delimiter[]);
/*parses a line and fills the "t_view" struct with data.
"line" is teh line, "delimiter" the delimiter string for every cell,
"buffer" is the buffer to put the distribution (min len is cells - distribution_offset).
"distribution_offset" is the number of cells before that the distribution starts,
(3 for now)*/
static int parse_line(char *line, char *delimiter, double *buffer,
		      unsigned int cells, unsigned int distribution_offset,
		      stochastic_task_view * t_view);


stochastic_taskset *load_stochastic_taskset(FILE * source,
					    char delimiter[])
{
    stochastic_taskset *taskset;
    stochastic_task_view t_view;
    size_t line_len = 0;
    ssize_t str_len = 0;
    double *buffer = NULL;
    char *line = NULL;
    unsigned int cells = 0;
    unsigned int distribution_offset = 3;

    taskset = new_stochastic_taskset(10);
    while ((str_len = getline(&line, &line_len, source)) != -1) {
	if (line[0] != '#') {


	    cells = get_cells(line, delimiter) - distribution_offset;

	    buffer = realloc(buffer, (cells) * sizeof(double));
	    if (!buffer) {
		return NULL;
	    }
	    memset(buffer, 0, (cells) * sizeof(double));

	    if (!parse_line
		(line, delimiter, buffer, cells, distribution_offset,
		 &t_view)) {
		free(buffer);
		free(line);

		free_stochastic_taskset(taskset);
		return NULL;
	    }

	    if (!add_task(taskset, t_view)) {
		free(buffer);
		free(line);
		free_stochastic_taskset(taskset);
		return NULL;
	    }
	}

    }
    free(buffer);
    free(line);

    return taskset;

}

int
fprintf_stochastic_distribution(FILE * destination,
				stochastic_distribution * sd)
{
    int bytes = 0;
    int i;
    for (i = 0; i < sd->d_len; i++) {

	bytes += fprintf(destination, "x:%i\tp:%lf\n", i, sd->dist[i]);
    }
    return bytes;
}

int fprintf_backlog_matrix(FILE * destination, backlog_matrix * b_mat)
{
    int count = 0;

    count += fprintf(destination, "dimension %zu\n", b_mat->dimension);
    {
	int i, j;
	for (i = 0; i < b_mat->dimension; i++) {
	    for (j = 0; j < b_mat->dimension; j++) {
		count +=
		    fprintf(destination, "%lf\t",
			    b_mat->matrix[j + (i * b_mat->dimension)]);
	    }
	    count += fprintf(destination, "\n");
	}
    }

    return count;

}

static int
parse_line(char *line, char *delimiter, double *buffer,
	   unsigned int cells, unsigned int distribution_offset,
	   stochastic_task_view * t_view)
{
    int actual_cell = 0;
    char *line_copy, *line_copy_start, *cell;
    unsigned int deadline, period, activation_time;

    line_copy = strdup(line);
    if (!line_copy) {
	return 0;
    }
    line_copy_start = line_copy;
    while ((cell = strsep(&line_copy, delimiter))) {
	switch (actual_cell) {
	case 0:
	    deadline = atoi(cell);
	    break;
	case 1:
	    period = atoi(cell);
	    break;
	case 2:
	    activation_time = atoi(cell);
	    break;
	default:;
	    break;
	}

	if (actual_cell >= distribution_offset) {
	    buffer[actual_cell - distribution_offset] = atof(cell);
	}

	actual_cell++;
    }

    free(line_copy_start);

    *t_view = new_stochastic_task_view(buffer,
				       cells,
				       deadline, period, activation_time);


    return 1;

}

static unsigned int get_cells(const char *string, const char delimiter[])
{
    unsigned int count, at_least_one;
    char *copy, *copy_to_free;

    copy = strdup(string);
    if (!copy)
	return 0;

    copy_to_free = copy;

    count = 0;
    at_least_one = 0;


    while (strsep(&copy, delimiter)) {
	count += 1;
    }
    free(copy_to_free);
    return count + at_least_one;
}
