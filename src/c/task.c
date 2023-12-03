#include <conf.h>

#define TASK_COLGEN_UPDATE         0
#define TASK_BACKTEST_FORKS_UPDATE 1
#define TASK_YAHOO_EOD_UPDATE      2
#define TASK_YAHOO_FIN_UPDATE      3

#define PRIO_HI     1
#define PRIO_LO     2

#define MAX_TASKS 100

void *task_yahoo_fin_update     (void *args);
void *task_yahoo_eod_update     (void *args);
void *task_quant_colgen_update  (void *args);
void *task_backtest_forks_update(void *args);

struct task **tasks;
int nr_tasks;

struct tasklet {
	tasklet_handler_t  handler;
	void              *args;
	int                seconds;   // run function every x seconds (if repeatable otherwise just once)
	time_t             timestamp; // unix timestamp of the last time the function was called
	bool               repeat;
};

struct tasklet_queue {
        struct tasklet             *tasklet;
        TAILQ_ENTRY(tasklet_queue)  tasklets;
};

typedef struct tasklet_queue        tasklet_queue_t;
typedef struct tasklet              tasklet_t;

TAILQ_HEAD(, tasklet_queue)         tasklet_queue_head;
mutex_t                             tasklet_lock;

/*
 * User Defined Tasks
 */
char *TASKS[] = {
		"STOCKS_QUANT_COLGEN_UPDATE",
		"STOCKS_BACKTEST_FORKS_UPDATE",
		"STOCKS_YAHOO_EOD_UPDATE",
		"STOCKS_YAHOO_FIN_UPDATE"
};

task_handler_t task_handlers[] =
{
	task_quant_colgen_update,
	task_backtest_forks_update,
	task_yahoo_eod_update,
	task_yahoo_fin_update,
};

void *task_backtest_forks_update(void *args)
{
	return (NULL);
	
}

// yahoo fundamentals update
void *task_yahoo_fin_update(void *args)
{
	return (NULL);
}

void *task_yahoo_earnings_update(void *args)
{
	return (NULL);
}

void *task_yahoo_eod_update(void *args)
{
	char *argv[] = { "/bin/sh", "-c", "/usr/bin/python3 /stockminer/src/python/stockminer/src/stockminer/stockminer.py yahoo 1>/dev/null", NULL };
	os_exec_argv(argv);
	return (NULL);
}

void *task_quant_colgen_update(void *args)
{	
	char *argv[] = { "/bin/sh", "-c", "/usr/bin/python3 /stockminer/src/python/stockminer/src/stockminer/stockminer.py colgen", NULL };
	os_exec_argv(argv);	
	return (NULL);
}

int TASK(char *tsk_name)
{
	for (int x = 0; x<sizeof(TASKS)/sizeof(char *); x++) {
		if (!strcmp(TASKS[x], tsk_name))
			return x;
	}
}

void task_benchmark(char *args)
{
	time_t before = time(NULL);
	switch (TASK(args)) {
		case TASK_COLGEN_UPDATE:
			break;
	}
	time_t after = time(NULL);
}

struct task *pick_task(int *workload)
{
	struct task *task;
	int ntasks = nr_tasks;

	for (int x = 0; x<ntasks; x++) {
		task = tasks[x];
		if (!task)
			continue;
		if (task->nr_miners < task->nr_preferred) {
			*workload = ATOMIC_INC(&task->nr_miners);
			return (task);
		}
	}
}

void task_schedule(void)
{
	struct tm  utc_tm;
	time_t     epoch;
	int        ntasks = nr_tasks;

	time(&epoch);
	gmtime_r(&epoch, &utc_tm);
	for (int x = 0; x<ntasks; x++) {
		struct task *task = tasks[x];
		if (!task)
			continue;
		if ((task->start_hour == (utc_tm.tm_hour%24)) && (task->start_min == utc_tm.tm_min))
			thread_create(task->handler, NULL);
	}
}


/*
 * Server receives a "request to mine" RPC
 *   - Server assigns a task to the miner such as
 *     downloading the EOD CSV OHLC data from yahoo
 *     for the stocks 1-200 (as per STOCKS.TXT)
 */
void apc_mine(struct connection *connection, char **argv)
{
	struct task *task;
	int          workload;

	task = pick_task(&workload);

}

void init_tasks(void)
{
	struct filemap filemap;
	struct task   *task;
	char           buf[16 KB];
	char          *argv[8];
	char          *line, *priority, *tz, *p;
	char          *taskname, *start_time, *start_day, *end_day;
	int            argc;

	tasks = zmalloc(sizeof(struct task *) * MAX_TASKS);
	TAILQ_INIT(&tasklet_queue_head);

	fs_readfile_str((char *)"config/tasks.csv", buf, sizeof(buf));
	line = buf;
	while ((p=strchr(line, '\n'))) {
		task = zmalloc(sizeof(*task));
		*p++ = 0;
		argc = cstring_split(line, argv, 6, ',');
		line = p;
		// Task Name
		taskname = argv[0];
		for (int x = 0; x<sizeof(TASKS)/sizeof(char *); x++) {
			if (!strcmp(TASKS[x], argv[0])) {
				taskname = argv[0];
				break;
			}
		}

		if (!taskname)
			continue;
		task->name    = strdup(taskname);
		task->handler = task_handlers[TASK(taskname)];

		// priority
		priority = argv[1];
		while (*priority == ' ')
			priority++;
		if (strcmp(priority, "PRIO_HI"))
			task->priority = PRIO_HI;
		else if (strcmp(priority, "PRIO_LO"))
			task->priority = PRIO_LO;
		else
			continue;

		// StartDay-LastDay (inclusive)
		start_day = argv[2];
		while (*start_day == ' ')
			start_day++;
		p = strchr(start_day, '-');
		if (!p)
			continue;
		*p++ = 0;
		end_day = p;

		task->start_day  = weekday_offset(start_day);
		task->end_day    = weekday_offset(end_day);

		// the "hour:minute timezone" when the task should be started
		task->start_hour = atoi(argv[3]);
		p = strchr(argv[3], ':');
		if (!p)
			continue;
		task->start_min  = atoi(p+1);

		tz = strchr(p+1, ' ');
		if (!tz)
			continue;
		task->start_hour += timezone_offset(tz+1);

		tasks[nr_tasks++] = task;
		if (nr_tasks >= MAX_TASKS)
			break;
	}
	task_schedule();
}

void *tasklet_free_XLS(void *args)
{
	struct XLS *XLS = (struct XLS *)args;

	for (int x = 0; x<XLS->nr_stocks; x++) {
		struct stock *stock = &XLS->STOCKS_ARRAY[x];
		free(stock->mag);
		if (stock->mag2)
			free(stock->mag2);
		if (stock->mag3)
			free(stock->mag3);
		free(stock);
	}
	free(XLS);
}

void tasklet_enqueue(tasklet_handler_t handler, void *args, int seconds, bool repeat)
{
	tasklet_queue_t *tasklet_queue_entry = zmalloc(sizeof(tasklet_queue_t));
	tasklet_t  *tasklet                  = zmalloc(sizeof(tasklet_t));

	if (!tasklet || !tasklet_queue_entry)
		return;

	tasklet->handler   = handler;
	tasklet->seconds   = seconds;
	tasklet->repeat    = repeat;
	tasklet->args      = args;
	tasklet->timestamp = time(NULL);
	tasklet_queue_entry->tasklet = tasklet;

	mutex_lock(&tasklet_lock);
	TAILQ_INSERT_TAIL(&tasklet_queue_head, tasklet_queue_entry, tasklets);
	mutex_unlock(&tasklet_lock);
}

void tasklet_schedule(void)
{
	struct tasklet_queue *tasklet_queue;
	struct tasklet       *tasklet;
	time_t                curtime = time(NULL);

	mutex_lock(&tasklet_lock);
	TAILQ_FOREACH(tasklet_queue, &tasklet_queue_head, tasklets) {
		tasklet = tasklet_queue->tasklet;
		if (curtime-tasklet->timestamp >= tasklet->seconds) {
			printf("tasklet func: %p\n", tasklet->handler);
			tasklet->handler(tasklet->args);
		}
	}
	mutex_unlock(&tasklet_lock);
}
