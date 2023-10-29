#ifndef __JOBS_H
#define __JOBS_H

#include <stdinc.h>

#ifndef KB
#define KB * 1024
#endif

#define JOB_SCHEDULER_POPEN     1 << 0
#define JOB_SCHEDULER_CONTAINER 1 << 1
#define JOB_SCHEDULER_KVM       1 << 2
#define JOB_READ_OUTPUT         1 << 11
#define MAX_CMDLINE_SIZE        32 KB

struct session;
struct job;

typedef void *(*job_exec_f)(struct job *job);
typedef void *(*job_comp_f)(struct job *job);

struct job {
	struct session *session;
	struct job     *jobchain;
	char           *exename;
	char           *exepath;
	char           *cmdline;
	char           *subcmd;
	char            output_filepath[256];
	char           *output;
	int64_t         output_size;
	uint64_t        options;
	job_exec_f      execute;     // void  (*execute)   (struct job *job);
	job_comp_f      completion;  // void *(*completion)(struct job *job);
};

void       *job_execute_popen    (struct job *job);
void       *job_execute_container(struct job *job);
void        job_exec_python      (char *python_path, char *python_script, char *nr_procs);
void        sched_exec_scrape    (int argc, char *argv[]);
struct job *new_job              (int options,job_comp_f completion);
void        init_webscript       (void);

#endif
