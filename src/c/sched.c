#include <conf.h>

struct job *new_job(int options, job_comp_f completion)
{
	struct job *job = (struct job *)zmalloc(sizeof(struct job));
	if (!job)
		return NULL;
	job->completion = completion;
	job->options    = options;
	if (job->options & JOB_SCHEDULER_POPEN)
		job->execute = (job_exec_f)job_execute_popen;
	else if (job->options & JOB_SCHEDULER_CONTAINER)
		job->execute = (job_exec_f)job_execute_container;
	return job;
}

/*
	if (argv[1] && !strcmp(argv[1], "python")) {
		if (!config_get("python_path", CONF_TYPE_STR, &Server.python_path, NULL))
			exit(-1);
		job_exec_python(Server.python_path, argv[2], argv[3]);
		exit(0);
	} else if (argv[1] && !strcmp(argv[1], "scrape"))
		sched_exec_scrape(argc, argv);
*/

void *job_execute_container(struct job *job)
{
	return NULL;
}

void *job_execute_popen(struct job *job)
{
	FILE *fp = popen(job->cmdline, "r");
	if (!fp)
		return NULL;
	pclose(fp);
	if (*job->output_filepath && (job->options & JOB_READ_OUTPUT))
		job->output = fs_mallocfile_str(job->output_filepath, &job->output_size);
	job->completion(job);
	return NULL;
}

void job_exec_python(char *python_path, char *python_script, char *nr_procs)
{
	char cmd[256];
	int x, pid, status, nr_python_processes;
printf("pythonpath: %s script: %s nr_procs: %s\n", python_path, python_script, nr_procs);
	if (!python_path || !python_script || !nr_procs)
		return;
	nr_python_processes = atoi(nr_procs);
printf("nr: %d\n", nr_python_processes);
	if (!nr_python_processes)
		return;

	for (x=0; x<nr_python_processes; x++) {
		switch ((pid=fork())) {
			case 0:
				snprintf(cmd, sizeof(cmd)-1, "python3 -W ignore %s/%s/%s.py %d", python_path, python_script, python_script, x);
				printf("executing: %s\n", cmd);
				system(cmd);
				exit(0);
			default:
				continue;
		}
	}
	while ((pid=wait(&status)) > 0);
}

const char *defense_gov_contracts[] = {
	"AIR FORCE",
	"ARMY",
	"NAVY",
	"DEFENSE LOGISTICS AGENCY",
	"DEFENSE HEALTH AGENCY",
	"DEFENSE THREAT REDUCTION AGENCY",
	"MISSILE DEFENSE AGENCY",
	"WASHINGTON HEADQUARTERS SERVICES",
	"U.S. TRANSPORTATION COMMAND"
};

#define MAX_PAGES              1024
#define MAX_CONTRACTS_PER_PAGE  256
#define DEFENSE_GOV_AIRFORCE     0
#define DEFENSE_GOV_ARMY         1
#define DEFENSE_GOV_NAVY         2
#define DEFENSE_GOV_DLA          3
#define NR_CONTRACT_TYPES        sizeof(defense_gov_contracts)/sizeof(char *)

#define CONTRACT_PATH_JSON       "data/military/defense.gov/contracts.json"
#define CONTRACT_PATH_TXT        "data/military/defense.gov/contracts.txt"

void sched_exec_scrape(int argc, char *argv[])
{
	struct stat     sb;
	in_addr_t       ipaddr;
	char            url[256];
	char            page[512 KB];
	char           *contracts[MAX_CONTRACTS_PER_PAGE]      = {0};
	char           *contract_dates[MAX_CONTRACTS_PER_PAGE] = {0};
	char           *contract_json = (char *)malloc(64 KB);
	char            contract_line[64 KB];
	char           *p, *p2, *p3, *p4, *category, *contract_name, *contract_desc, *contract_date, *contract_block;
	int             nr_pages, nr_contracts, contract_json_size = 1, max_json_size = 64 KB;
	int             category_index, start_page = 1, end_page, linesize, x, y, z;

	//  0              1      2        3=start_page  end_page(optional)
	// ./stockminer scrape defense.gov 2             4 (scrapes from page 2 to page 4) (4 arguments)
	// ./stockminer scrape defense.gov 4               (scrapes from page 1 to page 4) (3 arguments)
	if (!argv[2] || !argv[3])
		goto out;

	nr_pages = atoi(argv[3]);
	if (nr_pages < 0 || nr_pages >= MAX_PAGES)
		goto out;

	if (argv[4]) {
		end_page = atoi(argv[4]);
		if (!end_page) {
			printf(BOLDRED "end_page error" RESET "\n");
			goto out;
		}
		start_page = nr_pages;
		nr_pages   = (end_page - start_page) + 1; // page counts start from 1 so end page is inclusive
		if (nr_pages <= 0 || nr_pages >= MAX_PAGES) {
			printf(BOLDRED "exceeded max (%d) pages: (%d)" RESET "\n", MAX_PAGES, end_page);
			goto out;
		}
	}
	printf("start_page: %d end_page: %d nr_pages: %d\n", start_page, end_page, nr_pages);
	if (stat(CONTRACT_PATH_JSON, &sb) != -1) {
		truncate(CONTRACT_PATH_JSON, sb.st_size-1);
		contract_json[0] = ',';
	} else
		contract_json[0] = '[';

	for (x=0; x<nr_pages; x++) {
		snprintf(url, sizeof(url)-1, DEFENSE_GOV_PAGE, start_page++);
		printf("scraping url: %s start_page: %d\n", url, start_page);
		for (y=0; y<3; y++) {
			if (!curl_get(url, page)) {
				sleep(2);
				continue;
			} else
				break;
		}

		contract_name = strstr(page, "<feature-template temp");
		if (!contract_name)
			continue;

		// don't waste time scanning beyond the useful data
		p = strstr(contract_name, "</feature-template>");
		if (!p)
			continue;
		*p = 0;

		// Extract Contract URLs from the list of contracts on this GET request: ?Page=x
		// Each contract URL pertains to a different date
		nr_contracts = 0;
		while ((p=strstr(contract_name, "article-id="))) {
			contract_name = p+12;
			*(contract_name+7) = 0;
			printf(BOLDGREEN "contract: %s" RESET "\n", contract_name);
			contracts[nr_contracts] = strdup(contract_name);
			if (nr_contracts >= MAX_CONTRACTS_PER_PAGE)
				break;
			contract_name += 16;

			contract_date = strstr(contract_name, "publish-date-jss=");
			if (!contract_date)
				continue;
			contract_date      += 18;
			*(contract_date+10) = 0;
			contract_dates[nr_contracts++] = strdup(contract_date);
			contract_name = contract_date+16;
		}
		printf(BOLDCYAN "nr_contracts: %d" RESET "\n", nr_contracts);

		// Extract Contracts from each Contract URL
		for (y=0; y<nr_contracts; y++) {
			snprintf(url, sizeof(url)-1, DEFENSE_GOV_CONTRACT, contracts[y]);
			printf(BOLDRED "URL: %s" RESET "\n", url);
			for (z=0; z<3; z++) {
				if (!curl_get(url, page)) {
					printf(BOLDRED "Failed request: %s" RESET "\n", url);
					sleep(2);
					continue;
				} else
					break;
			}
			p = strstr(page, "content content-wrap");
			if (!p)
				continue;

			// null terminate the end of useful data
			p2 = strstr(p, "</div>");
			if (!p2)
				continue;
			*p2 = 0;

			// extract each contract subtype eg: <strong>Navy</strong>
			while ((category=strstr(p, "<strong>"))) {
				category += 8;
				contract_block = strchr(category, '<');
				if (!contract_block)
					break;
				*contract_block++ = 0;

				p2 = strstr(contract_block, "<p style");
				if (p2) {
					*(p2++) = 0;
					p = p2;
				}

				// contract category [AIRFORCE,ARMY,NAVY,...]
				for (z=0; z<NR_CONTRACT_TYPES; z++) {
					if (!strcmp(category, defense_gov_contracts[z])) {
						category_index = z;
						break;
					}
				}
				// extract each contract from this category
				while ((contract_desc=strstr(contract_block, "<p>"))) {
					p2 = strstr(contract_desc, "</p>");
					if (!p2)
						break;
					*p2++ = 0;
					contract_desc += 3;

					// *Small Business + **Mandatory Source filter 
					if (*contract_desc == '*') {
						char c1 = *(contract_desc+1);
						char c2 = *(contract_desc+2);
						if ((c1 == '*' && c2 == 'M') || (c1 == 'S')) {
							contract_block = p2;
							continue;
						}
					}

					// <br /> filter
					p3 = strstr(contract_desc, "<br />");
					if (p3) {
						*p3 = 0;
						contract_block = p2;
						continue;
					}

					// " -> ' filter
					p4 = contract_desc;
					while ((p3=strchr(p4, '\"'))) {
						*p3++ = '\'';
						p4 = p3;
					}

					if (contract_json_size+(64 KB) >= max_json_size) {
						max_json_size += 64 KB;
						contract_json  = (char *)realloc(contract_json, max_json_size);
						if (!contract_json)
							exit(-1);
					}
					contract_json_size += snprintf(contract_json+contract_json_size,sizeof(contract_line)-1,
					"{\"type\":\"%s\",\"date\":\"%s\",\"contract\":\"%s\"},", defense_gov_contracts[category_index], contract_dates[y], contract_desc);

					linesize = snprintf(contract_line, sizeof(contract_line)-1, "[%s:%s] %s\n", defense_gov_contracts[category_index], contract_dates[y], contract_desc);
					fs_appendfile(CONTRACT_PATH_TXT, contract_line, linesize);
					contract_block = p2;
				}
			} // <strong>
			sleep(1);
		} // nr_contracts
		sleep(1);
	} // nr_pages
	if (contract_json_size > 1) {
		contract_json[contract_json_size-1] = ']';
		fs_appendfile(CONTRACT_PATH_JSON, contract_json, contract_json_size);
	}
out:
	exit(0);
}
