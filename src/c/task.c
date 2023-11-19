#include <conf.h>

#define TASK_COLGEN_UPDATE         0
#define TASK_BACKTEST_FORKS_UPDATE 1
#define TASK_YAHOO_EOD_UPDATE      2
#define TASK_YAHOO_FIN_UPDATE      3

#define PRIORITY_HIGH   1
#define PRIORITY_MEDIUM 2
#define PRIORITY_LOW    3

void task_yahoo_fin_update     (void *args);
void task_yahoo_eod_update     (void *args);
void task_colgen_update        (void *args);
void task_backtest_forks_update(void *args);

struct task tasks[] =
{
	{ TASK_COLGEN_UPDATE,         PRIORITY_HIGH, task_colgen_update,         4},
	{ TASK_BACKTEST_FORKS_UPDATE, PRIORITY_HIGH, task_backtest_forks_update, 6},
	{ TASK_YAHOO_EOD_UPDATE,      PRIORITY_LOW,  task_yahoo_eod_update,      2},
	{ TASK_YAHOO_FIN_UPDATE,      PRIORITY_LOW,  task_yahoo_fin_update,      4}
};

void task_backtest_forks_update(void *args)
{

}

// yahoo fundamentals update
void task_yahoo_fin_update(void *args)
{



}


void task_yahoo_eod_update(void *args)
{


}

void task_colgen_update(void *args)
{


}

char *tasks_argv[] = { "colgen", "forks", "yahooEOD", "yahooFIN" };

int TASK(char *tsk_name)
{
	for (int x = 0; x<sizeof(tasks_argv)/sizeof(char *); x++) {
		if (!strcmp(tasks_argv[x], tsk_name))
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
	time_t after  = time(NULL);
}

struct task *pick_task(int *workload)
{
	struct task *task;
	for (int x = 0; x<sizeof(tasks)/sizeof(struct task); x++) {
		task = &tasks[x];
		if (task->nr_miners < task->nr_preferred) {
			*workload = ATOMIC_INC(&task->nr_miners);
			return (task);
		}
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
