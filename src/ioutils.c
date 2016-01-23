
#include "ioutils.h"

#include <stdlib.h>
#include <string.h>


static unsigned int get_cells (const char *string, const char delimiter[]);
static int parse_line (char *line, char *delimiter, double *buffer,
		       unsigned int cells, stochastic_task_view * t_view);


stochastic_taskset *
load_stochastic_taskset (FILE * source, char delimiter[])
{
  stochastic_taskset *taskset;
  stochastic_task_view t_view;
  size_t line_len = 0;
  ssize_t str_len = 0;
  double *buffer = NULL;
  char *line = NULL;
  unsigned int cells = 0;

  taskset = new_stochastic_taskset (10);
  while ((str_len = getline (&line, &line_len, source)) != -1)
    {
      //   printf ("here\n");

      cells = get_cells (line, delimiter) - 3;

      buffer = realloc (buffer, (cells) * sizeof (double));
      if (!buffer)
	{
	  return NULL;
	}
      memset (buffer, 0, (cells) * sizeof (double));

      if (!parse_line (line, delimiter, buffer, cells, &t_view))
	{
	  free (buffer);
	  free (line);

	  free_stochastic_taskset (taskset);
	}

      if (!add_task (taskset, t_view))
	{
	  free (buffer);
	  free (line);

	  free_stochastic_taskset (taskset);
	  return NULL;
	}

    }
  free (buffer);
  free (line);

  return taskset;

}


static int
parse_line (char *line, char *delimiter, double *buffer,
	    unsigned int cells, stochastic_task_view * t_view)
{
  int actual_cell = 0;
  char *line_copy, *line_copy_start, *cell;

  line_copy = strdup (line);
  if (!line_copy)
    {
      return 0;
    }
  line_copy_start = line_copy;
  while ((cell = strsep (&line_copy, delimiter)))
    {
      switch (actual_cell)
	{
	case 0:
	  t_view->deadline = atoi (cell);
	  break;
	case 1:
	  t_view->period = atoi (cell);
	  break;
	case 2:
	  t_view->activation_time = atoi (cell);
	  break;
	default:
	  {
	    buffer[actual_cell - 3] = atof (cell);
	  }
	  break;
	}
      actual_cell++;
    }

  free (line_copy_start);


  t_view->distribution = buffer;
  t_view->d_len = cells;


  return 1;

}

static unsigned int
get_cells (const char *string, const char delimiter[])
{
  unsigned int count, at_least_one;
  char *copy, *copy_to_free;

  copy = strdup (string);
  if (!copy)
    return 0;

  copy_to_free = copy;

  count = 0;
  at_least_one = 0;


  while (strsep (&copy, delimiter))
    {
      count += 1;
    }
  free (copy_to_free);
  return count + at_least_one;
}
